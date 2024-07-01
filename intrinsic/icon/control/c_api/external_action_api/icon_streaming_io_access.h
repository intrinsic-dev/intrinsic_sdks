// Copyright 2023 Intrinsic Innovation LLC
// Intrinsic Proprietary and Confidential
// Provided subject to written agreement between the parties.

#ifndef INTRINSIC_ICON_CONTROL_C_API_EXTERNAL_ACTION_API_ICON_STREAMING_IO_ACCESS_H_
#define INTRINSIC_ICON_CONTROL_C_API_EXTERNAL_ACTION_API_ICON_STREAMING_IO_ACCESS_H_

#include <type_traits>
#include <utility>

#include "intrinsic/icon/control/c_api/c_action_factory_context.h"
#include "intrinsic/icon/control/c_api/c_realtime_status.h"
#include "intrinsic/icon/control/c_api/c_streaming_io_realtime_access.h"
#include "intrinsic/icon/control/c_api/convert_c_realtime_status.h"
#include "intrinsic/icon/control/c_api/wrappers/streaming_io_wrapper.h"
#include "intrinsic/icon/control/streaming_io_types.h"
#include "intrinsic/icon/utils/realtime_status.h"
#include "intrinsic/icon/utils/realtime_status_macro.h"
#include "intrinsic/icon/utils/realtime_status_or.h"

namespace intrinsic::icon {

class IconStreamingIoAccess {
 public:
  IconStreamingIoAccess(
      XfaIconStreamingIoRealtimeAccess* realtime_access,
      XfaIconStreamingIoRealtimeAccessVtable realtime_access_vtable)
      : realtime_access_(realtime_access),
        realtime_access_vtable_(std::move(realtime_access_vtable)) {}

  // Polls a streaming input.
  // Returns nullptr if nothing has been written to the streaming input for `id`
  // since the last call to PollInput() (or if nothing has been written at all).
  // Returns NotFoundError if there is no streaming input for `id`.
  // Returns InvalidArgumentError if there is a streaming input for `id` with
  // available data, but its type is not `RealtimeT`.
  template <typename RealtimeT>
  RealtimeStatusOr<const RealtimeT*> PollInput(StreamingInputId id);

  // Copies `output` into a buffer that is then made available to the
  // non-realtime thread.
  // NOTE: Because this copies data, be careful about large outputs!
  //
  // Returns NotFound if there is no streaming output in our
  // RealtimeStreamingIoStorage.
  // Returns InvalidArgument if there is a streaming output, but it has a type
  // other than RealtimeT.
  template <typename RealtimeT, typename = std::enable_if_t<
                                    std::is_trivially_copyable_v<RealtimeT>>>
  RealtimeStatus WriteOutput(const RealtimeT& output);

 private:
  XfaIconStreamingIoRealtimeAccess* realtime_access_ = nullptr;
  XfaIconStreamingIoRealtimeAccessVtable realtime_access_vtable_;
};

template <typename RealtimeT>
RealtimeStatusOr<const RealtimeT*> IconStreamingIoAccess::PollInput(
    StreamingInputId id) {
  XfaIconRealtimeStatus status;
  const XfaIconStreamingInputType* streaming_input =
      realtime_access_vtable_.poll_input(realtime_access_, id.value(), &status);
  INTRINSIC_RT_RETURN_IF_ERROR(ToRealtimeStatus(status));
  return UnwrapStreamingInput<RealtimeT>(streaming_input);
}

template <typename RealtimeT, typename>
RealtimeStatus IconStreamingIoAccess::WriteOutput(const RealtimeT& output) {
  return ToRealtimeStatus(realtime_access_vtable_.write_output(
      realtime_access_,
      reinterpret_cast<const XfaIconStreamingOutputType*>(&output),
      sizeof(output)));
}

}  // namespace intrinsic::icon

#endif  // INTRINSIC_ICON_CONTROL_C_API_EXTERNAL_ACTION_API_ICON_STREAMING_IO_ACCESS_H_
