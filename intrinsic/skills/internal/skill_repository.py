# Copyright 2023 Intrinsic Innovation LLC

"""Provides access to a collection of skills."""

import abc
from typing import List

from intrinsic.skills.internal import runtime_data as rd
from intrinsic.skills.python import skill_interface as skl


class SkillRepository(metaclass=abc.ABCMeta):
  """Provides access to a collection of skills.

  Implementations of this class should be thread-safe and expect that all
  methods may be called concurrently without external synchronization.
  """

  @abc.abstractmethod
  def get_skill(self, skill_alias: str) -> skl.Skill:
    """Returns the skill instance corresponding to the provided alias.

    Implementation may differ in how a skill instance is produced on each
    call with the following contract: For repeated calls with the same alias,
    implementations always have to:
      * return the same implementation of Skill
      * and return completely independent objects (e.g., by creating new
        instances on every call)
      * or raise an error if no new object can be provided

    See the derived classes for additional information.

    Args:
      skill_alias: The skill's alias.

    Returns:
      A Skill instance that corresponds to skill_alias.
    """
    raise NotImplementedError('Method not implemented!')

  @abc.abstractmethod
  def get_skill_execute(self, skill_alias: str) -> skl.SkillExecuteInterface:
    """Returns the skill execute instance given the provided alias.

    Args:
      skill_alias: The skill's alias.

    Returns:
      The SkillExecuteInterface that corresponds to the skill_alias.
    """
    raise NotImplementedError('Method not implemented!')

  @abc.abstractmethod
  def get_skill_project(self, skill_alias: str) -> skl.SkillProjectInterface:
    """Returns the skill project instance given the provided alias.

    Args:
      skill_alias: The skill's alias.

    Returns:
      The SkillProjectInterface that corresponds to the skill_alias.
    """
    raise NotImplementedError('Method not implemented!')

  @abc.abstractmethod
  def get_skill_runtime_data(self, skill_alias: str) -> rd.SkillRuntimeData:
    """Returns the skill runtime data given the provided alias.

    Args:
      skill_alias: The skill's alias.

    Returns:
      The SkillRuntimeData that corresponds to the skill_alias.
    """
    raise NotImplementedError('Method not implemented!')

  @abc.abstractmethod
  def get_skill_aliases(self) -> List[str]:
    """Returns the aliases of all Skills registered to this repository."""
    raise NotImplementedError('Method not implemented!')


class InvalidSkillAliasError(ValueError):
  """The user provided an invalid skill alias to the repository."""

  def __init__(self, message: str):
    super().__init__(message)
