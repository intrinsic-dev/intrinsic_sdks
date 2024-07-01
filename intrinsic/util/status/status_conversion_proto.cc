// Copyright 2023 Intrinsic Innovation LLC

#include "intrinsic/util/status/status_conversion_proto.h"

#include <string>

#include "absl/status/status.h"
#include "absl/strings/cord.h"
#include "absl/strings/string_view.h"
#include "intrinsic/util/status/status.pb.h"

namespace intrinsic {

void SaveStatusToProto(const absl::Status& status,
                       intrinsic_proto::StatusProto* out) {
  out->Clear();
  out->set_code(status.raw_code());
  if (status.raw_code() == 0) {
    return;
  }
  out->set_message(std::string(status.message()));
  auto* payloads = out->mutable_payloads();
  status.ForEachPayload(
      [payloads](absl::string_view type_key, const absl::Cord& payload) {
        (*payloads)[std::string(type_key)] = static_cast<std::string>(payload);
      });
}

absl::Status MakeStatusFromProto(const intrinsic_proto::StatusProto& proto) {
  absl::Status status(static_cast<absl::StatusCode>(proto.code()),
                      proto.message());
  // Note: Using C++17 structured bindings instead of `entry` crashes Clang 6.0
  // on Ubuntu 18.04 (bionic).
  for (const auto& entry : proto.payloads()) {
    status.SetPayload(/*type_url=*/entry.first,
                      /*payload=*/absl::Cord(entry.second));
  }
  return status;
}

}  // namespace intrinsic
