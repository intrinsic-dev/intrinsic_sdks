// Copyright 2023 Intrinsic Innovation LLC
// Intrinsic Proprietary and Confidential
// Provided subject to written agreement between the parties.

#include "intrinsic/icon/control/c_api/external_action_api/testing/icon_slot_map_fake.h"

#include <memory>
#include <optional>
#include <utility>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/str_cat.h"
#include "intrinsic/icon/control/c_api/c_feature_interfaces.h"
#include "intrinsic/icon/control/c_api/c_realtime_slot_map.h"
#include "intrinsic/icon/control/c_api/external_action_api/icon_realtime_slot_map.h"
#include "intrinsic/icon/control/c_api/external_action_api/testing/loopback_fake_arm.h"
#include "intrinsic/icon/control/slot_types.h"
#include "intrinsic/icon/proto/types.pb.h"

namespace intrinsic::icon {

IconSlotMapFake::IconSlotMapFake() {}

absl::StatusOr<LoopbackFakeArm*> IconSlotMapFake::AddLoopbackFakeArmSlot(
    absl::string_view slot_name, std::optional<JointLimits> application_limits,
    std::optional<JointLimits> system_limits) {
  if (slot_name_to_data_.contains(slot_name)) {
    return absl::AlreadyExistsError(
        absl::StrCat("Cannot add multiple slots with name '", slot_name, "'"));
  }
  LoopbackFakeArm arm;
  if (application_limits.has_value()) {
    if (absl::Status s = arm.SetApplicationLimits(*application_limits);
        !s.ok()) {
      return s;
    }
  }
  if (system_limits.has_value()) {
    if (absl::Status s = arm.SetSystemLimits(*system_limits); !s.ok()) {
      return s;
    }
  }

  RealtimeSlotId slot_id(next_slot_index_);
  absl::StatusOr<::intrinsic_proto::icon::PartConfig> config =
      LoopbackFakeArm::GetPartConfig(absl::StrCat("fake_arm_", slot_id.value()),
                                     application_limits, system_limits);
  if (!config.ok()) {
    return config.status();
  }
  // Only increment next_slot_index after the last possible failure.
  next_slot_index_++;
  slot_id_to_slot_name_[slot_id] = slot_name;
  slot_name_to_data_.emplace(
      slot_name,
      std::make_unique<SlotInfoAndFakeArm>(SlotInfoAndFakeArm{
          .slot_info = {.config = config.value(), .slot_id = slot_id},
          .fake_arm = std::move(arm)}));

  return &slot_name_to_data_.at(slot_name)->fake_arm;
}

LoopbackFakeArm* IconSlotMapFake::GetFakeArmForSlot(
    absl::string_view slot_name) {
  auto slot_info_and_arm = slot_name_to_data_.find(slot_name);
  if (slot_info_and_arm == slot_name_to_data_.end()) {
    return nullptr;
  }
  return &slot_info_and_arm->second->fake_arm;
}

const LoopbackFakeArm* IconSlotMapFake::GetFakeArmForSlot(
    absl::string_view slot_name) const {
  auto slot_info_and_arm = slot_name_to_data_.find(slot_name);
  if (slot_info_and_arm == slot_name_to_data_.end()) {
    return nullptr;
  }
  return &slot_info_and_arm->second->fake_arm;
}

absl::StatusOr<SlotInfo> IconSlotMapFake::GetSlotInfoForSlot(
    absl::string_view slot_name) const {
  auto slot_info_and_arm = slot_name_to_data_.find(slot_name);
  if (slot_info_and_arm == slot_name_to_data_.end()) {
    return absl::NotFoundError(
        absl::StrCat("No Slot with name '", slot_name, "'"));
  }
  return slot_info_and_arm->second->slot_info;
}

absl::StatusOr<RealtimeSlotId> IconSlotMapFake::GetIdForSlot(
    absl::string_view slot_name) const {
  auto slot_info_and_arm = slot_name_to_data_.find(slot_name);
  if (slot_info_and_arm == slot_name_to_data_.end()) {
    return absl::NotFoundError(
        absl::StrCat("No Slot with name '", slot_name, "'"));
  }
  return slot_info_and_arm->second->slot_info.slot_id;
}

IconRealtimeSlotMap IconSlotMapFake::MakeIconRealtimeSlotMap() {
  return IconRealtimeSlotMap(reinterpret_cast<XfaIconRealtimeSlotMap*>(this),
                             GetCApiVtable(),
                             LoopbackFakeArm::GetFeatureInterfaceVtable());
}

IconConstRealtimeSlotMap IconSlotMapFake::MakeIconConstRealtimeSlotMap() const {
  return IconConstRealtimeSlotMap(
      reinterpret_cast<const XfaIconRealtimeSlotMap*>(this), GetCApiVtable(),
      LoopbackFakeArm::GetFeatureInterfaceVtable());
}

// static
XfaIconRealtimeSlotMapVtable IconSlotMapFake::GetCApiVtable() {
  return {
      .get_mutable_feature_interfaces_for_slot =
          [](XfaIconRealtimeSlotMap* self,
             uint64_t slot_id) -> XfaIconFeatureInterfacesForSlot {
        LoopbackFakeArm* arm_ptr = nullptr;
        auto slot_map = reinterpret_cast<IconSlotMapFake*>(self);
        auto slot_name =
            slot_map->slot_id_to_slot_name_.find(RealtimeSlotId(slot_id));
        if (slot_name != slot_map->slot_id_to_slot_name_.end()) {
          arm_ptr = slot_map->GetFakeArmForSlot(slot_name->second);
        }
        // Simply use the LoopbackFakeArm pointer for all feature
        // interfaces, since we know it supports them all. We need to
        // make sure to use an XfaIconFeatureInterfaceVtable struct whose
        // functions correctly cast this back to LoopbackFakeArm (see
        // MakeIconRealtimeSlotMap above)!
        return LoopbackFakeArm::MakeXfaIconFeatureInterfacesForSlot(arm_ptr);
      },
      .get_feature_interfaces_for_slot =
          [](const XfaIconRealtimeSlotMap* self,
             uint64_t slot_id) -> XfaIconConstFeatureInterfacesForSlot {
        const LoopbackFakeArm* arm_ptr = nullptr;
        auto slot_map = reinterpret_cast<const IconSlotMapFake*>(self);
        auto slot_name =
            slot_map->slot_id_to_slot_name_.find(RealtimeSlotId(slot_id));
        if (slot_name != slot_map->slot_id_to_slot_name_.end()) {
          arm_ptr = slot_map->GetFakeArmForSlot(slot_name->second);
        }
        // Simply use the LoopbackFakeArm pointer for all feature
        // interfaces, since we know it supports them all. We need to
        // make sure to use an XfaIconFeatureInterfaceVtable struct whose
        // functions correctly cast this back to LoopbackFakeArm (see
        // MakeIconConstRealtimeSlotMap above)!!
        return LoopbackFakeArm::MakeXfaIconConstFeatureInterfacesForSlot(
            arm_ptr);
      },
  };
}

}  // namespace intrinsic::icon
