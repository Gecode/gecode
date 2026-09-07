"""Minimal owning ctypes binding for Gecode's version 1 optimization ABI.

Set GECODE_OPTIMIZE_LIBRARY to the shared C ABI library, or pass Library(path).
No native dependency is downloaded, installed, or loaded until explicitly used.
"""
from .binding import (ApiError, Backend, Cancellation, Guarantee, GlobalConstraint, Indicator, Library, Model,
                      Options, Result, Row, Session, Termination, Variable,
                      VariableType, load_library)
from .binding import (PoolAttempt, PoolCompletion, PoolEntry, PoolOptions, PoolResult,
                      RelaxationSelection, RelaxationSide, RepairItem, RepairOptions,
                      RepairResult, RepairStage, RepairVariable, Validation)

__all__ = ["ApiError", "Backend", "Cancellation", "Guarantee", "GlobalConstraint", "Indicator", "Library", "Model",
           "Options", "Result", "Row", "Session", "Termination", "Variable",
           "VariableType", "load_library"]
__all__ += ["PoolAttempt", "PoolCompletion", "PoolEntry", "PoolOptions", "PoolResult",
            "RelaxationSelection", "RelaxationSide", "RepairItem", "RepairOptions",
            "RepairResult", "RepairStage", "RepairVariable", "Validation"]

from .binding import VariableSpec, RowSpec, SparseRowBatch
__all__ += ["VariableSpec", "RowSpec", "SparseRowBatch"]

from .binding import QuadraticModel, QuadraticOptions, QuadraticResult, QuadraticChecks, WeightedSquare
__all__ += ["QuadraticModel", "QuadraticOptions", "QuadraticResult", "QuadraticChecks", "WeightedSquare"]

from .binding import (LpObservationOptions, LpObservationState, LpObservationReason,
                      LpBasisStatus, LpDualSource, LpObservationCapabilities,
                      LpObservationGroup, LpObservationMetadata, LpKktReport,
                      LpRowObservation, LpColumnObservation, LpObservations, LpObservedResult)
__all__ += ["LpObservationOptions", "LpObservationState", "LpObservationReason",
            "LpBasisStatus", "LpDualSource", "LpObservationCapabilities",
            "LpObservationGroup", "LpObservationMetadata", "LpKktReport",
            "LpRowObservation", "LpColumnObservation", "LpObservations", "LpObservedResult"]

from .binding import LpBasis, LpBasisInfo, LpBasisOrigin, LpBasisSubmissionState, LpBasisSubmission, LpBasisSolveResult
__all__ += ["LpBasis", "LpBasisInfo", "LpBasisOrigin", "LpBasisSubmissionState", "LpBasisSubmission", "LpBasisSolveResult"]

from .binding import RegularTransition
__all__ += ["RegularTransition"]

from .binding import (ScenarioReuse, ScenarioRunState, ScenarioBatchCompletion, ScenarioId,
                      ScenarioVariableBounds, ScenarioRowBounds, ScenarioDefinition, ScenarioOptions,
                      ScenarioStatistics, ScenarioBatchInfo, ScenarioResultInfo, ScenarioCheck,
                      ScenarioOutcome, ScenarioBatchResult)
__all__ += ["ScenarioReuse", "ScenarioRunState", "ScenarioBatchCompletion", "ScenarioId",
            "ScenarioVariableBounds", "ScenarioRowBounds", "ScenarioDefinition", "ScenarioOptions",
            "ScenarioStatistics", "ScenarioBatchInfo", "ScenarioResultInfo", "ScenarioCheck",
            "ScenarioOutcome", "ScenarioBatchResult"]

from .binding import (LpEvidenceRequest, LpEvidenceState, LpEvidenceReason, LpEvidenceCompletion,
                      LpEvidencePhase, LpEvidenceSide, LpEvidenceColumnKind, LpEvidenceOptions,
                      LpEvidenceInfo, LpEvidenceGroup, LpEvidenceMetadata, LpPrimalEvidence,
                      LpFarkasEvidence, LpEvidenceSlot, LpEvidenceDiagnostics, LpEvidenceRawResult,
                      LpEvidenceStageInfo, LpEvidenceColumn, LpEvidenceRawValue, LpEvidenceResult, LpEvidenceStage)
__all__ += ["LpEvidenceRequest", "LpEvidenceState", "LpEvidenceReason", "LpEvidenceCompletion",
            "LpEvidencePhase", "LpEvidenceSide", "LpEvidenceColumnKind", "LpEvidenceOptions",
            "LpEvidenceInfo", "LpEvidenceGroup", "LpEvidenceMetadata", "LpPrimalEvidence",
            "LpFarkasEvidence", "LpEvidenceSlot", "LpEvidenceDiagnostics", "LpEvidenceRawResult",
            "LpEvidenceStageInfo", "LpEvidenceColumn", "LpEvidenceRawValue", "LpEvidenceResult", "LpEvidenceStage"]

from .binding import (LpSensitivityState, LpSensitivityReason, LpSensitivityCompletion, LpRangeEndKind,
                      LpSensitivitySide, LpObjectiveParameter, LpEqualityRhsParameter, LpSensitivityTolerances,
                      LpSensitivityLimits, LpSensitivityOptions, LpSensitivityInfo, LpSensitivityWork,
                      LpSensitivityGroup, LpRangeEnd, LpSensitivityLimiter, LpIntervalCheckReport,
                      LpParameterInterval, LpSensitivityEntry, LpSensitivityReferenceChecks, LpSensitivityResult)
__all__ += ['LpSensitivityState', 'LpSensitivityReason', 'LpSensitivityCompletion', 'LpRangeEndKind', 'LpSensitivitySide', 'LpObjectiveParameter', 'LpEqualityRhsParameter', 'LpSensitivityTolerances', 'LpSensitivityLimits', 'LpSensitivityOptions', 'LpSensitivityInfo', 'LpSensitivityWork', 'LpSensitivityGroup', 'LpRangeEnd', 'LpSensitivityLimiter', 'LpIntervalCheckReport', 'LpParameterInterval', 'LpSensitivityEntry', 'LpSensitivityReferenceChecks', 'LpSensitivityResult']
