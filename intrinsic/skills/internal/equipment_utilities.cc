// Copyright 2023 Intrinsic Innovation LLC

#include "intrinsic/skills/internal/equipment_utilities.h"

#include <string>
#include <utility>
#include <vector>

#include "absl/container/flat_hash_map.h"
#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_join.h"
#include "google/protobuf/repeated_ptr_field.h"
#include "intrinsic/resources/proto/resource_handle.pb.h"
#include "intrinsic/skills/proto/equipment.pb.h"
#include "intrinsic/skills/proto/footprint.pb.h"

namespace intrinsic::skills {

absl::StatusOr<google::protobuf::RepeatedPtrField<
    intrinsic_proto::skills::ResourceReservation>>
ReserveEquipmentRequired(
    const absl::flat_hash_map<std::string,
                              intrinsic_proto::skills::ResourceSelector>&
        equipment_required,
    const google::protobuf::Map<std::string,
                                intrinsic_proto::resources::ResourceHandle>&
        resource_handles) {
  google::protobuf::RepeatedPtrField<
      intrinsic_proto::skills::ResourceReservation>
      resources;
  std::vector<std::string> missing_handles;
  for (const auto& [name, selector] : equipment_required) {
    auto handles_iter = resource_handles.find(name);
    if (handles_iter == resource_handles.end()) {
      missing_handles.push_back(name);
      continue;
    }
    // Handle is present so create a Resource.
    intrinsic_proto::skills::ResourceReservation resource;
    resource.set_type(selector.sharing_type());
    // `name` would be the skill internal name, but we need to reserve the
    // "external" name of the equiment.
    resource.set_name(handles_iter->second.name());
    *resources.Add() = std::move(resource);
  }
  if (!missing_handles.empty()) {
    return absl::InvalidArgumentError(
        absl::StrCat("Error when specifying equipment resources. "
                     "The Skill requires (",
                     absl::StrJoin(missing_handles, ", "),
                     ") but no handle was found with this name."));
  }
  return resources;
}

}  // namespace intrinsic::skills
