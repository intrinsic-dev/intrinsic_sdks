// Copyright 2023 Intrinsic Innovation LLC
// Intrinsic Proprietary and Confidential
// Provided subject to written agreement between the parties.

#include "intrinsic/icon/control/c_api/external_action_api/sine_wave_plugin_action.h"

#include <algorithm>
#include <cmath>
#include <memory>
#include <optional>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "google/protobuf/duration.pb.h"
#include "intrinsic/icon/actions/action_utils.h"
#include "intrinsic/icon/control/c_api/external_action_api/sine_wave_action.pb.h"
#include "intrinsic/icon/control/streaming_io_types.h"
#include "intrinsic/icon/proto/generic_part_config.pb.h"
#include "intrinsic/icon/proto/types.pb.h"
#include "intrinsic/icon/release/status_helpers.h"
#include "intrinsic/icon/utils/realtime_status.h"
#include "intrinsic/icon/utils/realtime_status_or.h"
#include "intrinsic/kinematics/types/joint_limits.h"
#include "intrinsic/util/proto_time.h"

namespace intrinsic::icon {

namespace {
// This is provided to the OutputWriter to convert from the data type used in
// the realtime loop (double in this case) to the proto message type that
// arrives in clients (google::protobuf::Duration).
//
// The OutputWriter calls this function in the non-realtime thread.
absl::StatusOr<google::protobuf::Duration> DoubleToDuration(double seconds) {
  google::protobuf::Duration proto;
  if (absl::Status s = ::intrinsic::ToProto(absl::Seconds(seconds), &proto);
      !s.ok()) {
    return s;
  }
  return proto;
}

absl::StatusOr<float> FloatFromProto(
    const SineWavePluginAction::StreamingInputProto& input) {
  return input.value();
}

}  // namespace

absl::StatusOr<SineWavePluginAction::SolvedParams>
SineWavePluginAction::SolvedParams::FromProto(
    const SineWavePluginAction::ParameterProto& proto_params) {
  SineWavePluginAction::SolvedParams params;
  for (const auto& joint_params : proto_params.joints()) {
    params.amplitudes.emplace_back(joint_params.amplitude_rad());
    params.frequencies.emplace_back(joint_params.frequency_hz());
  }
  if (!params.IsValid(proto_params.joints_size())) {
    return absl::InternalError(
        "Params object has inconsistent number of values.");
  }
  return params;
}

bool SineWavePluginAction::SolvedParams::IsValid(size_t ndof) const {
  return amplitudes.size() == ndof && frequencies.size() == ndof;
}

intrinsic_proto::icon::ActionSignature SineWavePluginAction::GetSignature() {
  ActionSignatureBuilder b(SineWavePluginAction::kName, "Waves a Sine!");
  (void)b.AddPartSlot(SineWavePluginAction::kSlotName, "an arm",
                      /*required_feature_interfaces=*/
                      {
                          intrinsic_proto::icon::FeatureInterfaceTypes::
                              FEATURE_INTERFACE_JOINT_LIMITS,
                          intrinsic_proto::icon::FeatureInterfaceTypes::
                              FEATURE_INTERFACE_JOINT_POSITION,
                          intrinsic_proto::icon::FeatureInterfaceTypes::
                              FEATURE_INTERFACE_JOINT_POSITION_SENSOR,
                      });
  (void)b.SetFixedParametersType<SineWavePluginAction::ParameterProto>();
  (void)b.AddStateVariable<
      intrinsic_proto::icon::ActionSignature::StateVariableInfo::TYPE_DOUBLE>(
      kStateVariableTimeSinceStart, "seconds since the Action was started.");
  (void)b.AddStateVariable<
      intrinsic_proto::icon::ActionSignature::StateVariableInfo::TYPE_DOUBLE>(
      kStateVariableNumber,
      "reports the latest value received via the streaming input 'number'.");
  (void)b.AddStreamingInput<StreamingInputProto>(
      kStreamingInputName,
      "a number that is echoed back via the 'number' state variable");
  (void)b.AddStreamingOutput<StreamingOutputProto>(
      "time_since_start", "Reports the time since the Action was started.");
  return b.Finish();
}

absl::StatusOr<std::unique_ptr<SineWavePluginAction>>
SineWavePluginAction::Create(const ParameterProto& parameters,
                             IconActionFactoryContext& context) {
  LOG(INFO) << "PUBLIC: Attempt to create SineWavePluginAction.";
  INTRINSIC_ASSIGN_OR_RETURN(
      auto arm_info, context.GetSlotInfo(SineWavePluginAction::kSlotName));
  if (!arm_info.config.generic_config().has_joint_limits_config()) {
    return absl::InvalidArgumentError("SineWaveAction requires joint limits.");
  }
  const ::intrinsic_proto::icon::GenericJointLimitsConfig& joint_limits_config =
      arm_info.config.generic_config().joint_limits_config();

  INTRINSIC_ASSIGN_OR_RETURN(
      JointLimits application_limits,
      ::intrinsic::FromProto(joint_limits_config.application_limits()));

  // Advertise a streaming output that takes a double in the realtime thread
  // (see the call to OutputWriter::Write() below), and converts it to a
  // Duration proto in the non-realtime thread using the supplied callback.
  //
  // Extra parentheses because the ASSIGN_OR_RETURN macro would otherwise get
  // confused by the template typename and method call, since those contain
  // commas.
  INTRINSIC_RETURN_IF_ERROR(
      (context.AddStreamingOutputConverter<double, google::protobuf::Duration>(
          &DoubleToDuration)));

  // Similar for the streaming input, except the conversion goes the other way:
  // ICON invokes `FloatFromProto` in the non-realtime thread to convert a proto
  // message to a C++ data type (in this case, a float), which ICON then makes
  // available to the realtime thread.
  INTRINSIC_ASSIGN_OR_RETURN(
      StreamingInputId streaming_input_id,
      (context.AddStreamingInputParser<StreamingInputProto, float>(
          kStateVariableNumber, &FloatFromProto)));

  // Convert the proto parameters to SolvedParams, which is safe to read in
  // realtime.
  INTRINSIC_ASSIGN_OR_RETURN(SolvedParams params,
                             SolvedParams::FromProto(parameters));

  LOG(INFO) << "PUBLIC: Created SineWavePluginAction.";
  return std::make_unique<SineWavePluginAction>(
      arm_info.slot_id, application_limits,
      context.ServerConfig().frequency_hz(), params, streaming_input_id);
}

RealtimeStatus SineWavePluginAction::OnEnter(
    const IconConstRealtimeSlotMap& slot_map) {
  current_state_ = std::nullopt;
  return icon::OkStatus();
}

RealtimeStatus SineWavePluginAction::Sense(
    const IconConstRealtimeSlotMap& slot_map,
    IconStreamingIoAccess& io_access) {
  std::optional<const IconConstFeatureInterfaces> feature_interfaces =
      slot_map.FeatureInterfacesForSlot(slot_id_);
  if (!feature_interfaces.has_value()) {
    return InternalError(RealtimeStatus::StrCat(
        "Failed to get FeatureInterfaces for Slot ", kSlotName));
  }
  if (!feature_interfaces->joint_position.has_value()) {
    return InternalError("Slot does not have JointPosition");
  }

  if (!current_state_.has_value()) {
    current_state_ = {
        .time_since_start = 0.0,
        .starting_position =
            feature_interfaces->joint_position->PreviousPositionSetpoints()
                .position(),
        .current_number = std::nullopt,
    };
  } else {
    current_state_->time_since_start += 1.0 / frequency_hz_;
  }

  RealtimeStatusOr<const float*> number =
      io_access.PollInput<float>(number_id_);
  if (number.ok() && number.value() != nullptr) {
    current_state_->current_number = *number.value();
  }

  // Submit the current value of time_since_start_ to the OutputWriter. Note
  // that our conversion function (DoubleToDuration()) is only invoked in the
  // non-realtime thread, when a client actually requests the output data.
  //
  // On the first cycle, calling Write() will also unblock any clients that
  // are waiting for output data from this Action.
  return io_access.WriteOutput<double>(current_state_->time_since_start);
}

RealtimeStatus SineWavePluginAction::Control(IconRealtimeSlotMap& slot_map) {
  std::optional<IconFeatureInterfaces> feature_interfaces =
      slot_map.MutableFeatureInterfacesForSlot(slot_id_);
  if (!feature_interfaces.has_value()) {
    return InternalError(RealtimeStatus::StrCat(
        "Failed to get FeatureInterfaces for Slot ", kSlotName));
  }
  if (!feature_interfaces->joint_position.has_value()) {
    return InternalError("Slot does not have JointPosition");
  }
  eigenmath::VectorNd position_reference(
      current_state_->starting_position.size());
  for (size_t i = 0; i < current_state_->starting_position.size(); ++i) {
    double sine_value = std::sin(current_state_->time_since_start * 2. * M_PI *
                                 params_.frequencies[i]);
    double sine_cubed = sine_value * sine_value * sine_value;
    position_reference(i) = current_state_->starting_position(i) +
                            params_.amplitudes[i] * sine_cubed;
    // Clamp position reference to joint limits. This doesn't prevent
    // exceeding the other limits (velocity, acceleration, jerk)!
    position_reference(i) =
        std::clamp(position_reference(i), application_limits_.min_position(i),
                   application_limits_.max_position(i));
  }
  return feature_interfaces->joint_position->SetPositionSetpoints(
      JointPositionCommand(position_reference));
}

RealtimeStatusOr<StateVariableValue> SineWavePluginAction::GetStateVariable(
    absl::string_view name) const {
  if (!current_state_.has_value()) {
    return UnavailableError("Action not initialized.");
  }
  if (name == kStateVariableTimeSinceStart) {
    return StateVariableValue(current_state_->time_since_start);
  }
  if (name == kStateVariableNumber) {
    if (!current_state_->current_number.has_value()) {
      return UnavailableError("No number written to streaming input yet.");
    }
    return StateVariableValue(current_state_->current_number.value());
  }
  return NotFoundError(RealtimeStatus::StrCat(
      "SineWaveAction, state variable not found '", name, "'"));
}

}  // namespace intrinsic::icon
