// Copyright 2023 Intrinsic Innovation LLC

#ifndef INTRINSIC_SKILLS_CC_GET_FOOTPRINT_REQUEST_H_
#define INTRINSIC_SKILLS_CC_GET_FOOTPRINT_REQUEST_H_

#include <optional>
#include <string>
#include <utility>

#include "absl/log/check.h"
#include "absl/status/statusor.h"
#include "absl/strings/string_view.h"
#include "google/protobuf/any.pb.h"
#include "google/protobuf/descriptor.h"
#include "google/protobuf/message.h"
#include "intrinsic/util/proto/any.h"

namespace intrinsic {
namespace skills {

// A request for a call to SkillInterface::GetFootprint.
class GetFootprintRequest {
 public:
  // `param_defaults` can specify default parameter values to merge into any
  // unset fields of `params`.
  explicit GetFootprintRequest(
      const ::google::protobuf::Message& params,
      ::google::protobuf::Message* param_defaults = nullptr) {
    params_any_.PackFrom(params);
    if (param_defaults != nullptr) {
      param_defaults_any_ = google::protobuf::Any();
      param_defaults_any_->PackFrom(*param_defaults);
    }
  }

  // Defers conversion of input Any params to target proto type until accessed
  // by the user in params().
  //
  // This constructor enables conversion from Any to the target type without
  // needing a message pool/factory up front, since params() is templated on the
  // target type.
  explicit GetFootprintRequest(
      google::protobuf::Any params,
      std::optional<::google::protobuf::Any> param_defaults)
      : params_any_(std::move(params)),
        param_defaults_any_(std::move(param_defaults)) {}

  // The skill parameters proto.
  template <class TParams>
  absl::StatusOr<TParams> params() const {
    return UnpackAnyAndMerge<TParams>(params_any_, param_defaults_any_);
  }

 private:
  ::google::protobuf::Any params_any_;
  std::optional<::google::protobuf::Any> param_defaults_any_;
};

}  // namespace skills
}  // namespace intrinsic

#endif  // INTRINSIC_SKILLS_CC_GET_FOOTPRINT_REQUEST_H_
