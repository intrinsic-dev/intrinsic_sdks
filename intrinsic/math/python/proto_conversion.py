# Copyright 2023 Intrinsic Innovation LLC

"""Converters from intrinsic math protos to commonly used in-memory representations."""

import sys

from intrinsic.math.proto import array_pb2
from intrinsic.math.proto import matrix_pb2
from intrinsic.math.proto import point_pb2
from intrinsic.math.proto import pose_pb2
from intrinsic.math.proto import quaternion_pb2
from intrinsic.math.proto import vector3_pb2
from intrinsic.math.python import data_types
import numpy as np


def ndarray_from_proto(array_proto: array_pb2.Array) -> np.ndarray:
  """Converts Array proto to np.ndarray."""
  if (
      array_proto.byte_order == array_pb2.Array.NO_BYTE_ORDER
      and array_proto.type not in _NO_BYTE_ORDER_SCALAR_TYPES
  ):
    raise ValueError(
        'Cannot specify `byte_order` = NO_BYTE_ORDER for arrays of '
        f'type {array_proto.type}.'
    )

  # Get the base dtype (which doesn't account for the byte order).
  try:
    dtype = _SCALAR_TYPE_TO_DTYPE[array_proto.type]
  except KeyError as err:
    raise ValueError(f'Unrecognized scalar type: {array_proto.type}') from err

  # Update it to account for the byte order.
  try:
    np_byte_order = _PROTO_BYTE_ORDER_TO_NUMPY[array_proto.byte_order]
  except KeyError as err:
    raise ValueError(
        f'Unrecognized proto byte order: {array_proto.byte_order}'
    ) from err
  else:
    dtype = dtype.newbyteorder(np_byte_order)

  return np.frombuffer(array_proto.data, dtype=dtype).reshape(array_proto.shape)


def ndarray_to_proto(array: np.ndarray) -> array_pb2.Array:
  """Converts np.ndarray to Array proto."""
  try:
    scalar_type = _NP_TYPE_TO_SCALAR_TYPE[array.dtype.type]
  except KeyError as err:
    raise ValueError(f'Unrecognized numpy type: {array.dtype.type}') from err

  if array.dtype.byteorder == '=':  # Native byte order.
    try:
      byte_order = _SYS_BYTE_ORDER_TO_PROTO[sys.byteorder]
    except KeyError as err:
      raise ValueError(
          f'Unrecognized sys byte order: {sys.byteorder!r}'
      ) from err
  else:
    try:
      byte_order = _NUMPY_BYTE_ORDER_TO_PROTO[array.dtype.byteorder]
    except KeyError as err:
      raise ValueError(
          f'Unrecognized numpy byte order: {array.dtype.byteorder!r}'
      ) from err

  return array_pb2.Array(
      data=array.tobytes(),
      shape=array.shape,
      type=scalar_type,
      byte_order=byte_order,
  )


def ndarray_from_matrix_proto(proto: matrix_pb2.Matrixd) -> np.ndarray:
  """Converts a matrix_pb2.Matrixd to a np.ndarray.

  Args:
    proto: The matrix as proto.

  Raises:
    ValueError: If the number of values in the proto do not match the spicified
      size.
  Returns:
    The matrix as np.ndarray.
  """
  if len(proto.values) != proto.rows * proto.cols:
    raise ValueError(
        f'matrix is not {proto.rows}x{proto.cols}, it has'
        f' {len(proto.values)} values.'
    )
  return np.array(proto.values).reshape(proto.rows, proto.cols)


def ndarray_to_matrix_proto(matrix: np.ndarray) -> matrix_pb2.Matrixd:
  """Converts a np.ndarray to a matrix_pb2.Matrixd.

  Args:
    matrix: The matrix as numpy array.

  Raises:
    ValueError: If the input is not a 2D array.
  Returns:
    The matrix as matrix_pb2.Matrixd.
  """
  if len(matrix.shape) != 2:
    raise ValueError(f'expected a 2D array, got shape {matrix.shape}.')
  return matrix_pb2.Matrixd(
      rows=matrix.shape[0], cols=matrix.shape[1], values=matrix.flatten()
  )


def ndarray_from_point_proto(point_proto: point_pb2.Point) -> np.ndarray:
  """Convert a point_pb2.Point to a size 3 np.ndarray."""
  return np.array([point_proto.x, point_proto.y, point_proto.z])


def ndarray_to_point_proto(point: np.ndarray) -> point_pb2.Point:
  """Converts a size 3 np.ndarray to a point_pb2.Point.

  Args:
    point: An np.ndarray of size 3.

  Returns:
    A point_pb2.Point.

  Raises:
    ValueError if the input array has not a length of 3.
  """
  if point.shape != (3,):
    raise ValueError(
        'Received point of size {0} but expected a size of 3.'.format(
            point.size
        )
    )
  return point_pb2.Point(x=point[0], y=point[1], z=point[2])


def quaternion_from_proto(
    quaternion_proto: quaternion_pb2.Quaternion,
) -> data_types.Quaternion:
  """Convert a quaternion proto to a quaternion."""
  return data_types.Quaternion(
      [
          quaternion_proto.x,
          quaternion_proto.y,
          quaternion_proto.z,
          quaternion_proto.w,
      ],
      normalize=False,
  )


def quaternion_to_proto(
    quat: data_types.Quaternion,
) -> quaternion_pb2.Quaternion:
  """Convert a quaternion to a quaternion proto."""
  return quaternion_pb2.Quaternion(x=quat.x, y=quat.y, z=quat.z, w=quat.w)


def pose_from_proto(pose_proto: pose_pb2.Pose) -> data_types.Pose3:
  """Convert a pose proto to a pose."""
  point = ndarray_from_point_proto(pose_proto.position)
  rotation = data_types.Rotation3(
      quat=quaternion_from_proto(pose_proto.orientation)
  )
  # We expect the quaternion in the input proto to be normalized. Pose3 does not
  # require this so we check this explicitly.
  rotation.quaternion.check_normalized()
  return data_types.Pose3(translation=point, rotation=rotation)


def pose_to_proto(pose: data_types.Pose3) -> pose_pb2.Pose:
  """Convert a pose to a pose proto."""
  msg = pose_pb2.Pose()
  msg.position.CopyFrom(ndarray_to_point_proto(pose.translation))
  # Normalize quaternion if this is not already the case (don't re-normalize and
  # introduce numerical variations). Pose3 may contain a non-unit quaternion.
  quat = pose.quaternion
  if not quat.is_normalized():
    quat = quat.normalize()
  msg.orientation.CopyFrom(quaternion_to_proto(quat))
  return msg


# Maps between Array.ScalarType and the corresponding numpy dtype.
_SCALAR_TYPE_TO_DTYPE = {
    array_pb2.Array.ScalarType.BOOL_SCALAR_TYPE: np.dtype('bool'),
    array_pb2.Array.ScalarType.INT8_SCALAR_TYPE: np.dtype('int8'),
    array_pb2.Array.ScalarType.INT16_SCALAR_TYPE: np.dtype('int16'),
    array_pb2.Array.ScalarType.INT32_SCALAR_TYPE: np.dtype('int32'),
    array_pb2.Array.ScalarType.INT64_SCALAR_TYPE: np.dtype('int64'),
    array_pb2.Array.ScalarType.UINT8_SCALAR_TYPE: np.dtype('uint8'),
    array_pb2.Array.ScalarType.UINT16_SCALAR_TYPE: np.dtype('uint16'),
    array_pb2.Array.ScalarType.UINT32_SCALAR_TYPE: np.dtype('uint32'),
    array_pb2.Array.ScalarType.UINT64_SCALAR_TYPE: np.dtype('uint64'),
    array_pb2.Array.ScalarType.FLOAT16_SCALAR_TYPE: np.dtype('float16'),
    array_pb2.Array.ScalarType.FLOAT32_SCALAR_TYPE: np.dtype('float32'),
    array_pb2.Array.ScalarType.FLOAT64_SCALAR_TYPE: np.dtype('float64'),
}
_NP_TYPE_TO_SCALAR_TYPE = {
    dtype.type: scalar_type
    for scalar_type, dtype in _SCALAR_TYPE_TO_DTYPE.items()
}

# Scalar types for which byte order does not apply (because they are single byte
# or fewer).
_NO_BYTE_ORDER_SCALAR_TYPES = frozenset({
    array_pb2.Array.ScalarType.BOOL_SCALAR_TYPE,
    array_pb2.Array.ScalarType.INT8_SCALAR_TYPE,
    array_pb2.Array.ScalarType.UINT8_SCALAR_TYPE,
})

# Maps between Array.ByteOrder and the corresponding numpy string.
_PROTO_BYTE_ORDER_TO_NUMPY = {
    array_pb2.Array.NO_BYTE_ORDER: '|',
    array_pb2.Array.ByteOrder.LITTLE_ENDIAN_BYTE_ORDER: '<',
    array_pb2.Array.ByteOrder.BIG_ENDIAN_BYTE_ORDER: '>',
}
_NUMPY_BYTE_ORDER_TO_PROTO = {
    numpy_byte_order: proto_byte_order
    for proto_byte_order, numpy_byte_order in _PROTO_BYTE_ORDER_TO_NUMPY.items()
}

# Maps from sys.byteorder values to Array.ByteOrder.
_SYS_BYTE_ORDER_TO_PROTO = {
    'little': array_pb2.Array.ByteOrder.LITTLE_ENDIAN_BYTE_ORDER,
    'big': array_pb2.Array.ByteOrder.BIG_ENDIAN_BYTE_ORDER,
}
