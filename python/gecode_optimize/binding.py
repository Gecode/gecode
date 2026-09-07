"""Owning Python wrappers for the versioned C ABI; close handles explicitly."""

import ctypes as C
from ctypes.util import find_library
from ._runtime import bundled_library_path
from dataclasses import dataclass, field
from enum import IntEnum
import math
import os
import threading
import time


class VariableType(IntEnum):
    CONTINUOUS=0; INTEGER=1; BINARY=2; SEMI_CONTINUOUS=3; SEMI_INTEGER=4
class Backend(IntEnum):
    AUTO=0; HIGHS=1; NATIVE=2
class Guarantee(IntEnum):
    NUMERICAL=0; EXACT=1; CERTIFIED=2
class Termination(IntEnum):
    UNKNOWN=0; OPTIMAL=1; INFEASIBLE=2; UNBOUNDED=3; INFEASIBLE_OR_UNBOUNDED=4
    TIME_LIMIT=5; NODE_LIMIT=6; MEMORY_LIMIT=7; ITERATION_LIMIT=8; SOLUTION_LIMIT=9
    OBJECTIVE_LIMIT=10; CANCELLED=11; NUMERICAL_FAILURE=12; UNSUPPORTED=13
    INVALID_MODEL=14; BACKEND_ERROR=15
class PoolCompletion(IntEnum):
    INCOMPLETE=0; REQUESTED_LIMIT=1; EXHAUSTED=2
class RelaxationSide(IntEnum):
    LOWER=0; UPPER=1


class ApiError(RuntimeError):
    """C ABI misuse/model-building error; distinct from solver Termination."""
    def __init__(self, code, message):
        self.code = code
        super().__init__(message)


U64=C.c_uint64; I64=C.c_int64; I32=C.c_int32; U32=C.c_uint32; F64=C.c_double; U8=C.c_uint8
class _Id(C.Structure):
    _fields_=[("model_id", U64), ("slot", U64), ("kind", U32), ("reserved", U32)]
class _RegularTransition(C.Structure):
    _fields_=[("struct_size",U64),("reserved",U64),("from_state",U64),("symbol",I64),("to_state",U64)]
class _Term(C.Structure):
    _fields_=[("variable", _Id), ("coefficient", F64)]
class _Start(C.Structure):
    _fields_=[("variable", _Id), ("value", F64)]
class _VariableSpec(C.Structure):
    _fields_=[("struct_size",U64),("type",I32),("reserved",I32),
              ("lower",F64),("upper",F64),("name",C.c_char_p)]
class _RowSpec(C.Structure):
    _fields_=[("struct_size",U64),("reserved",U64),("terms",C.POINTER(_Term)),
              ("term_count",U64),("lower",F64),("upper",F64),("name",C.c_char_p)]
class _SparseRowBatch(C.Structure):
    _fields_=[("struct_size",U64),("reserved",U64),
              ("columns",C.POINTER(_Id)),("columns_count",U64),
              ("row_start",C.POINTER(U64)),("row_start_count",U64),
              ("column",C.POINTER(U64)),("column_count",U64),
              ("coefficient",C.POINTER(F64)),("coefficient_count",U64),
              ("lower",C.POINTER(F64)),("lower_count",U64),
              ("upper",C.POINTER(F64)),("upper_count",U64),
              ("names",C.POINTER(C.c_char_p)),("names_count",U64)]
class _Options(C.Structure):
    _fields_=[("struct_size", U64), ("backend", I32), ("guarantee", I32), ("threads", I32), ("random_seed", I32),
              ("time_limit_seconds", F64), ("relative_gap", F64), ("absolute_gap", F64),
              ("feasibility_tolerance", F64), ("integrality_tolerance", F64), ("node_limit", U64),
              ("has_node_limit", I32), ("reserved", I32), ("cancellation", U64),
              ("primal_start", C.POINTER(_Start)), ("primal_start_count", U64)]
class _Info(C.Structure):
    _fields_=[("model_id", U64), ("revision", U64), ("variable_slots", U64), ("termination", I32),
              ("guarantee", I32), ("has_solution", I32), ("solution_validated", I32), ("start_submitted", I32),
              ("reserved", I32), ("elapsed_seconds", F64)]
class _Statistics(C.Structure):
    _fields_=[(name, U64) for name in ("solve_calls", "model_loads", "incremental_updates", "unchanged_models", "basis_warm_starts", "incumbent_starts")]
class _OptionalNumber(C.Structure):
    _fields_=[("present", I32), ("reserved", I32), ("value", F64)]
class _LpOptions(C.Structure):
    _fields_=[("struct_size",U64),("reserved",U64),("solve",_Options),("duals",I32),("basis",I32)]+[
        (name,F64) for name in ("dual_feasibility","stationarity","complementarity","objective_gap")]
class _LpCapabilities(C.Structure):
    _fields_=[("struct_size",U64),("reserved",U64)]+[(name,I32) for name in
        ("available","duals","basis_export","reserved_flags")]+[("limitation_count",U64)]
class _LpInfo(C.Structure):
    _fields_=[("struct_size",U64),("reserved",U64),("result",_Info),("has_observations",I32),("reserved_flags",I32)]+[
        (name,U64) for name in ("model_id","revision","row_slots","column_slots")]
class _BasisInfo(C.Structure):
    _fields_=[(name,U64) for name in ("struct_size","reserved","model_id","revision","row_slots","column_slots")]+[
        ("origin",I32),("reserved_flags",I32)]
class _BasisResultInfo(C.Structure):
    _fields_=[("struct_size",U64),("reserved",U64),("result",_Info),
              ("requested_model_id",U64),("requested_revision",U64)]+[(name,I32) for name in
        ("has_requested_basis","state","backend_attempted","has_statuses_changed","statuses_changed","reserved_flags")]
class _LpMetadata(C.Structure):
    _fields_=[("struct_size",U64),("reserved",U64)]+[(name,F64) for name in
        ("dual_feasibility","stationarity","complementarity","objective_gap","primal_check_tolerance")]+[
        (name,_OptionalNumber) for name in ("backend_primal_tolerance","backend_dual_tolerance")]
class _LpGroup(C.Structure):
    _fields_=[("struct_size",U64),("reserved",U64),("state",I32),("reason",I32)]
class _LpRow(C.Structure):
    _fields_=[("struct_size",U64),("reserved",U64)]+[(name,I32) for name in
        ("active","dual_source","has_basis","basis")]+[(name,_OptionalNumber) for name in
        ("activity","lower_slack","upper_slack","dual")]
class _LpColumn(C.Structure):
    _fields_=[("struct_size",U64),("reserved",U64)]+[(name,I32) for name in
        ("active","has_basis","basis","reserved_flags")]+[("reduced_cost",_OptionalNumber)]
class _LpChecks(C.Structure):
    _fields_=[("struct_size",U64),("reserved",U64)]+[(name,I32) for name in
        ("primal_valid","dual_signs_valid","stationarity_valid","complementarity_valid","gap_valid","accepted")]+[
        (name,_OptionalNumber) for name in ("max_dual_sign_violation","max_stationarity","max_complementarity",
                                          "dual_objective_estimate","normalized_gap")]
class _WeightedSquare(C.Structure):
    _fields_=[("struct_size",U64),("reserved",U64),("terms",C.POINTER(_Term)),
              ("term_count",U64),("offset",F64),("weight",F64),("name",C.c_char_p)]
class _QuadraticOptions(C.Structure):
    _fields_=[("struct_size",U64),("reserved",U64),("solve",_Options)]+[
        (name,U64) for name in ("iteration_limit","max_auxiliary_variables","max_lifted_nonzeros")]+[
        (name,F64) for name in ("stationarity_tolerance","complementarity_tolerance","optimality_tolerance")]
class _QuadraticInfo(C.Structure):
    _fields_=[("result",_Info),("qp_iterations",U64),("regularization",F64)]
class _QuadraticChecks(C.Structure):
    _fields_=[(name,I32) for name in ("primal_valid","objective_valid","kkt_available","kkt_valid","bound_valid","reserved")]+[
        (name,_OptionalNumber) for name in ("max_stationarity","max_complementarity","original_objective",
                                           "normalized_lower_bound","gap_upper_bound")]+[
        ("square_count",U64),("gradient_slots",U64)]
class _PoolOptions(C.Structure):
    _fields_=[("struct_size", U64), ("solve", _Options), ("max_solutions", U64),
              ("has_projection", I32), ("reserved", I32), ("projection", C.POINTER(_Id)), ("projection_count", U64)]
class _PoolInfo(C.Structure):
    _fields_=[(name,U64) for name in ("model_id","revision","projection_count","entry_count","attempt_count","ranked_prefix")]+[
        (name,I32) for name in ("termination","completion","guarantee","reserved")]+[("elapsed_seconds",F64)]
class _PoolEntryInfo(C.Structure):
    _fields_=[("projection_count",U64),("rank_established",I32),("reserved",I32)]
class _PoolAttemptInfo(C.Structure):
    _fields_=[(name,I32) for name in ("termination","guarantee","candidate_accepted","rank_established")]+[
        ("objective",_OptionalNumber),("remaining_bound",_OptionalNumber)]
class _Selection(C.Structure):
    _fields_=[("source",_Id),("side",I32),("reserved",I32),("penalty",F64)]
class _RepairOptions(C.Structure):
    _fields_=[("struct_size",U64),("solve",_Options),("selections",C.POINTER(_Selection)),
              ("selection_count",U64),("optimize_original_objective",I32),("reserved",I32)]
class _RepairInfo(C.Structure):
    _fields_=[(name,U64) for name in ("source_model_id","source_revision","private_model_id","private_revision",
        "variable_slots","item_count","stage_count","completed_stages")]+[(name,I32) for name in (
        "termination","guarantee","has_private_model","has_repair","minimum_violation_established",
        "original_objective_optimized","workflow_termination","workflow_guarantee")]+[
        ("elapsed_seconds",F64),("workflow_elapsed_seconds",F64)]
class _RepairItemInfo(C.Structure):
    _fields_=[("source",_Id),("slack",_Id),("penalty_row",_Id),("side",I32),("reserved",I32),
              ("original_bound",F64),("penalty",F64)]+[(name,_OptionalNumber) for name in (
              "activity","violation","weighted_violation","slack_value")]
class _RepairStageInfo(C.Structure):
    _fields_=[("index",U64),("completed",I32),("reserved",I32),("retention_bound",_OptionalNumber)]
class _ValidationInfo(C.Structure):
    _fields_=[("valid",I32),("model_valid",I32),("violated_globals",U64)]+[(name,F64) for name in (
        "max_bound_violation","max_row_violation","max_integrality_violation","max_indicator_violation")]+[("objective",_OptionalNumber)]


class _ScenarioId(C.Structure):
    _fields_=[("batch_id",U64),("index",U64)]
class _ScenarioBounds(C.Structure):
    _fields_=[("struct_size",U64),("reserved",U64),("entity",_Id),("has_lower",I32),("has_upper",I32),("lower",F64),("upper",F64)]
class _ScenarioDefinition(C.Structure):
    _fields_=[("struct_size",U64),("reserved",U64),("name",C.c_char_p),
              ("objective_coefficients",C.POINTER(_Term)),("objective_count",U64),("objective_offset",_OptionalNumber),
              ("variable_bounds",C.POINTER(_ScenarioBounds)),("variable_count",U64),
              ("row_bounds",C.POINTER(_ScenarioBounds)),("row_count",U64)]
class _ScenarioOptions(C.Structure):
    _fields_=[("struct_size",U64),("reserved",U64),("solve",_Options),("reuse",I32),("reserved_flags",I32)]+[
        (name,U64) for name in ("max_scenarios","max_patch_entries","max_saved_value_slots","max_work")]
class _ScenarioInfo(C.Structure):
    _fields_=[(name,U64) for name in ("struct_size","reserved","model_id","revision","batch_id","scenario_count","outcome_count")]+[
        (name,I32) for name in ("has_batch","completion","has_stop_reason","stop_reason","has_offending_scenario","all_resolved")]+[
        (name,U64) for name in ("offending_scenario","attempted","resolved","work")]+[("elapsed_seconds",F64),("reuse_statistics",_Statistics)]
class _ScenarioOutcome(C.Structure):
    _fields_=[("struct_size",U64),("reserved",U64),("scenario",_ScenarioId)]+[
        (name,I32) for name in ("state","has_result","has_check","reserved_flags")]+[
        ("result",_Info),("reuse_delta",_Statistics),("elapsed_seconds",F64)]
class _ScenarioCheck(C.Structure):
    _fields_=[("struct_size",U64),("reserved",U64)]+[(name,I32) for name in
        ("has_check","identity_valid","candidate_examined","objective_matches","exact_witness_validated","reserved_flags")]+[("validation",_ValidationInfo)]
class _ScenarioDefinitionInfo(C.Structure):
    _fields_=[(name,U64) for name in ("struct_size","reserved","objective_count","variable_count","row_count")]+[("objective_offset",_OptionalNumber)]


class _EvidenceOptions(C.Structure):
    _fields_=[("struct_size",U64),("reserved",U64),("solve",_Options),("request",I32),("reserved_flags",I32)]+[
        (name,F64) for name in ("recession","stationarity","minimum_improvement","minimum_contradiction")]+[
        (name,U64) for name in ("max_auxiliary_variables","max_auxiliary_rows","max_auxiliary_nonzeros","max_retained_slots","max_work","max_auxiliary_solves")]
class _EvidenceInfo(C.Structure):
    _fields_=[(name,U64) for name in ("struct_size","reserved","model_id","revision")]+[
        (name,I32) for name in ("has_evidence","completion","has_stop_reason","stop_reason")]+[
        (name,U64) for name in ("row_slots","column_slots","stage_count","attempted_calls","work")]+[("elapsed_seconds",F64)]
class _EvidenceGroup(C.Structure):
    _fields_=[("struct_size",U64),("reserved",U64),("state",I32),("reason",I32)]
class _EvidenceMetadata(C.Structure):
    _fields_=[("struct_size",U64),("reserved",U64)]+[(name,F64) for name in
        ("recession","stationarity","minimum_improvement","minimum_contradiction","primal_tolerance")]
class _EvidencePrimal(C.Structure):
    _fields_=[("struct_size",U64),("reserved",U64),("has_base_check",I32),("reserved_flags",I32),("base_check",_ValidationInfo)]+[
        (name,_OptionalNumber) for name in ("direction_scale","normalized_objective_slope","max_variable_recession_violation","max_row_recession_violation")]
class _EvidenceFarkas(C.Structure):
    _fields_=[("struct_size",U64),("reserved",U64)]+[(name,_OptionalNumber) for name in
        ("multiplier_scale","contradiction_margin","max_stationarity")]
class _EvidenceSlot(C.Structure):
    _fields_=[("struct_size",U64),("reserved",U64),("source",_Id)]+[(name,I32) for name in
        ("active","has_side","side","reserved_flags")]+[(name,_OptionalNumber) for name in
        ("base_value","direction","multiplier","contribution","selected_bound")]
class _EvidenceRawResult(C.Structure):
    _fields_=[(name,U64) for name in ("struct_size","reserved","model_id","revision","value_count","mask_count")]+[
        (name,I32) for name in ("termination_code","guarantee_code","reported_solution_validated","reported_start_submitted")]+[
        ("elapsed_seconds",F64)]+[(name,_OptionalNumber) for name in ("objective","best_bound","absolute_gap","relative_gap","native_gap")]
class _EvidenceStage(C.Structure):
    _fields_=[(name,U64) for name in ("struct_size","reserved","index","private_model_id","private_revision","row_count","column_count","nonzeros")]+[
        (name,I32) for name in ("phase","attempted","has_raw_result","candidate_examined")]+[("check",_ValidationInfo),("raw_result",_EvidenceRawResult)]
class _EvidenceColumn(C.Structure):
    _fields_=[("struct_size",U64),("reserved",U64),("private_variable",_Id),("source",_Id)]+[
        (name,I32) for name in ("kind","has_side","side","reserved_flags")]
class _EvidenceRawValue(C.Structure):
    _fields_=[("struct_size",U64),("reserved",U64),("slot",U64),("reported_value",_OptionalNumber),("has_reported_mask",I32),("reported_mask",I32)]



class _SensitivityRequest(C.Structure):
    _fields_=[("struct_size",U64),("reserved",U64),("kind",I32),("reserved_flags",I32),("entity",_Id)]
class _SensitivityChecksOptions(C.Structure):
    _fields_=[("struct_size",U64),("reserved",U64)]+[(name,F64) for name in
        ("primal_feasibility","dual_feasibility","stationarity","complementarity","objective_gap","system_absolute","system_relative")]
class _SensitivityLimits(C.Structure):
    _fields_=[(name,U64) for name in ("struct_size","reserved","max_rows","max_columns","max_nonzeros","max_requests",
        "max_basis_solves","max_factor_entries","max_retained_slots","max_work")]
class _SensitivityOptions(C.Structure):
    _fields_=[("struct_size",U64),("reserved",U64),("backend",I32),("reserved_flags",I32),
        ("time_limit_seconds",F64),("cancellation",U64),("checks",_SensitivityChecksOptions),("limits",_SensitivityLimits),
        ("requests",C.POINTER(_SensitivityRequest)),("request_count",U64)]
class _SensitivityInfo(C.Structure):
    _fields_=[(name,U64) for name in ("struct_size","reserved","model_id","revision")]+[(name,I32) for name in
        ("completion","reason","has_stop_reason","stop_reason","has_sensitivity","has_basis","guarantee","reserved_flags")]+[
        (name,U64) for name in ("entry_count","factor_order_count","row_slots","column_slots")]+[("elapsed_seconds",F64)]
class _SensitivityWork(C.Structure):
    _fields_=[("struct_size",U64),("reserved",U64),("factor_setup_attempted",I32),("reserved_flags",I32)]+[
        (name,U64) for name in ("basis_solves","coordinator_visits","retained_slots","preparation_visits")]
class _SensitivityGroup(C.Structure):
    _fields_=[("struct_size",U64),("reserved",U64),("state",I32),("reason",I32)]
class _SensitivityEnd(C.Structure):
    _fields_=[("struct_size",U64),("reserved",U64),("kind",I32),("reserved_flags",I32),("value",_OptionalNumber)]
class _SensitivityLimiter(C.Structure):
    _fields_=[("struct_size",U64),("reserved",U64),("entity",_Id),("side",I32),("dual_condition",I32)]
class _SensitivityIntervalChecks(C.Structure):
    _fields_=[("struct_size",U64),("reserved",U64),("inequalities",U64)]+[(name,I32) for name in
        ("accepted","lower_direction_checked","upper_direction_checked","reserved_flags")]+[("max_endpoint_violation",_OptionalNumber)]
class _SensitivityEntry(C.Structure):
    _fields_=[("struct_size",U64),("reserved",U64),("request",_SensitivityRequest),("group",_SensitivityGroup),("index",U64)]+[
        (name,I32) for name in ("requested","has_interval","has_lower_limiter","has_upper_limiter")]+[
        ("anchor",F64),("lower",_SensitivityEnd),("upper",_SensitivityEnd),("objective_slope",_OptionalNumber),
        ("lower_limiter",_SensitivityLimiter),("upper_limiter",_SensitivityLimiter),("checks",_SensitivityIntervalChecks)]
class _SensitivityReferenceChecks(C.Structure):
    _fields_=[("struct_size",U64),("reserved",U64),("primal",_ValidationInfo),("kkt",_LpChecks),
        ("basis_point_matches",I32),("reserved_flags",I32)]+[(name,_OptionalNumber) for name in
        ("max_point_difference","max_system_residual","max_scaled_system_residual")]


def _integer(value, bits, signed=False):
    if isinstance(value, bool) or not isinstance(value, int):
        raise TypeError("integer argument must be an int, not a float/bool")
    low=-(1 << (bits-1)) if signed else 0
    high=(1 << (bits-1))-1 if signed else (1 << bits)-1
    if not low <= value <= high:
        raise OverflowError("integer does not fit the C ABI field")
    return value


def _text(value):
    if not isinstance(value, str):
        raise TypeError("expected str")
    if "\0" in value:
        raise ValueError("embedded NUL would truncate the C string")
    return value.encode("utf-8")


class Library:
    """One explicitly loaded ABI instance. Keep handles within this instance."""
    def __init__(self, path):
        self.path=os.fspath(path)
        self._dll=C.CDLL(self.path)
        self._dll.gecode_opt_v1_abi_version.argtypes=[]
        self._dll.gecode_opt_v1_abi_version.restype=U32
        if self._dll.gecode_opt_v1_abi_version() != 1:
            raise RuntimeError("unsupported Gecode optimization C ABI version")
        self._dll.gecode_opt_v1_last_error.argtypes=[]
        self._dll.gecode_opt_v1_last_error.restype=C.c_char_p
        ptr=C.POINTER
        signatures={
            "sensitivity_options_default": [ptr(_SensitivityOptions),U64],
            "analyze_lp_sensitivity": [U64,ptr(_SensitivityOptions),ptr(U64)],
            "sensitivity_info": [U64,ptr(_SensitivityInfo),U64],
            "sensitivity_work": [U64,ptr(_SensitivityWork),U64],
            "sensitivity_checks_options": [U64,ptr(_SensitivityChecksOptions),U64],
            "sensitivity_reference_checks": [U64,ptr(_SensitivityReferenceChecks),U64],
            "sensitivity_copy_source_observed": [U64,ptr(U64)],
            "sensitivity_copy_basis": [U64,ptr(U64)],
            "sensitivity_entry": [U64,U64,ptr(_SensitivityEntry),U64],
            "sensitivity_entries": [U64,ptr(_SensitivityEntry),U64,U64,ptr(U64)],
            "sensitivity_objective": [U64,_Id,ptr(_SensitivityEntry),U64],
            "sensitivity_equality_rhs": [U64,_Id,ptr(_SensitivityEntry),U64],
            "sensitivity_factor_order": [U64,ptr(_Id),U64,ptr(U64)],
            "sensitivity_active_slots": [U64,I32,ptr(U8),U64,ptr(U64)],
            "sensitivity_text": [U64,I32,U64,ptr(C.c_char),U64,ptr(U64)],
            "evidence_options_default": [ptr(_EvidenceOptions),U64],
            "analyze_lp_evidence": [U64,ptr(_EvidenceOptions),ptr(U64)],
            "lp_evidence_info": [U64,ptr(_EvidenceInfo),U64],
            "lp_evidence_group": [U64,I32,ptr(_EvidenceGroup),U64],
            "lp_evidence_metadata": [U64,ptr(_EvidenceMetadata),U64],
            "lp_evidence_primal": [U64,ptr(_EvidencePrimal),U64],
            "lp_evidence_farkas": [U64,ptr(_EvidenceFarkas),U64],
            "lp_evidence_slot": [U64,_Id,ptr(_EvidenceSlot),U64],
            "lp_evidence_slots": [U64,I32,ptr(_EvidenceSlot),U64,U64,ptr(U64)],
            "lp_evidence_value": [U64,_Id,I32,ptr(F64)],
            "lp_evidence_multiplier": [U64,_Id,ptr(_EvidenceSlot),U64],
            "lp_evidence_text": [U64,I32,ptr(C.c_char),U64,ptr(U64)],
            "lp_evidence_copy_stage": [U64,U64,ptr(U64)],
            "lp_evidence_stage_info": [U64,ptr(_EvidenceStage),U64],
            "lp_evidence_stage_columns": [U64,ptr(_EvidenceColumn),U64,U64,ptr(U64)],
            "lp_evidence_stage_raw_values": [U64,ptr(_EvidenceRawValue),U64,U64,ptr(U64)],
            "lp_evidence_stage_text": [U64,I32,ptr(C.c_char),U64,ptr(U64)],
            "scenario_options_default": [ptr(_ScenarioOptions),U64],
            "solve_scenarios": [U64,ptr(_ScenarioDefinition),U64,U64,ptr(_ScenarioOptions),ptr(U64)],
            "scenario_batch_info": [U64,ptr(_ScenarioInfo),U64],
            "scenario_batch_id": [U64,U64,ptr(_ScenarioId)],
            "scenario_batch_outcome": [U64,_ScenarioId,ptr(_ScenarioOutcome),U64],
            "scenario_batch_check": [U64,_ScenarioId,ptr(_ScenarioCheck),U64],
            "scenario_batch_copy_result": [U64,_ScenarioId,ptr(U64)],
            "scenario_batch_map": [U64,_Id,ptr(_Id)],
            "scenario_batch_value": [U64,_ScenarioId,_Id,ptr(F64)],
            "scenario_batch_message": [U64,ptr(C.c_char),U64,ptr(U64)],
            "scenario_batch_text": [U64,_ScenarioId,I32,ptr(C.c_char),U64,ptr(U64)],
            "scenario_batch_definition": [U64,_ScenarioId,ptr(_ScenarioDefinitionInfo),U64],
            "scenario_batch_objective": [U64,_ScenarioId,ptr(_Term),U64,ptr(U64)],
            "scenario_batch_bounds": [U64,_ScenarioId,I32,ptr(_ScenarioBounds),U64,U64,ptr(U64)],
            "basis_from_observed": [U64,ptr(U64)],
            "basis_from_model": [U64,ptr(I32),U64,ptr(I32),U64,ptr(U64)],
            "basis_info": [U64,ptr(_BasisInfo),U64],
            "basis_statuses": [U64,I32,ptr(I32),U64,ptr(U64)],
            "basis_row": [U64,_Id,ptr(I32)], "basis_column": [U64,_Id,ptr(I32)],
            "solve_lp_with_basis": [U64,U64,ptr(_LpOptions),ptr(U64)],
            "session_solve_lp_with_basis": [U64,U64,U64,ptr(_LpOptions),ptr(U64)],
            "basis_result_info": [U64,ptr(_BasisResultInfo),U64],
            "basis_result_message": [U64,ptr(C.c_char),U64,ptr(U64)],
            "basis_result_copy_observed": [U64,ptr(U64)], "basis_result_copy_basis": [U64,ptr(U64)],
            "lp_capabilities": [ptr(_LpCapabilities),U64],
            "lp_capability_text": [I32,U64,ptr(C.c_char),U64,ptr(U64)],
            "lp_options_default": [ptr(_LpOptions),U64],
            "solve_lp_observed": [U64,ptr(_LpOptions),ptr(U64)],
            "session_solve_lp_observed": [U64,U64,ptr(_LpOptions),ptr(U64)],
            "lp_observed_result_copy_result": [U64,ptr(U64)],
            "lp_observed_result_info": [U64,ptr(_LpInfo),U64],
            "lp_observed_result_metadata": [U64,ptr(_LpMetadata),U64],
            "lp_observed_result_group": [U64,I32,ptr(_LpGroup),U64],
            "lp_observed_result_checks": [U64,ptr(_LpChecks),U64],
            "lp_observed_result_row": [U64,_Id,ptr(_LpRow),U64],
            "lp_observed_result_column": [U64,_Id,ptr(_LpColumn),U64],
            "lp_observed_result_rows": [U64,ptr(_LpRow),U64,U64,ptr(U64)],
            "lp_observed_result_columns": [U64,ptr(_LpColumn),U64,U64,ptr(U64)],
            "lp_observed_result_text": [U64,I32,ptr(C.c_char),U64,ptr(U64)],
            "quadratic_capabilities": [ptr(I32)],
            "quadratic_options_default": [ptr(_QuadraticOptions),U64],
            "quadratic_model_create": [ptr(U64)], "quadratic_model_identity": [U64,ptr(U64),ptr(U64)],
            "quadratic_model_add_continuous": [U64,F64,F64,C.c_char_p,ptr(_Id)],
            "quadratic_model_add_row": [U64,ptr(_Term),U64,F64,F64,C.c_char_p,ptr(_Id)],
            "quadratic_model_set_objective": [U64,ptr(_WeightedSquare),U64,ptr(_Term),U64,I32,F64],
            "quadratic_model_set_variable_bounds": [U64,_Id,F64,F64],
            "quadratic_model_set_row_bounds": [U64,_Id,F64,F64],
            "quadratic_model_set_coefficient": [U64,_Id,_Id,F64],
            "quadratic_model_remove_variable": [U64,_Id], "quadratic_model_remove_row": [U64,_Id],
            "quadratic_solve": [U64,ptr(_QuadraticOptions),ptr(U64)],
            "quadratic_result_info": [U64,ptr(_QuadraticInfo),U64],
            "quadratic_result_checks": [U64,ptr(_QuadraticChecks),U64],
            "quadratic_result_number": [U64,I32,ptr(I32),ptr(F64)],
            "quadratic_result_value": [U64,_Id,ptr(F64)],
            "quadratic_result_values": [U64,ptr(F64),ptr(U8),ptr(U8),U64,ptr(U64)],
            "quadratic_result_array": [U64,I32,ptr(F64),U64,ptr(U64)],
            "quadratic_result_text": [U64,I32,ptr(C.c_char),U64,ptr(U64)],
            "options_default": [ptr(_Options), U64], "capabilities": [I32, ptr(I32), ptr(I32), ptr(I32)],
            "model_create": [ptr(U64)], "model_identity": [U64, ptr(U64), ptr(U64)],
            "model_read": [C.c_char_p, ptr(U64)], "model_write": [U64, C.c_char_p],
            "model_add_variable": [U64, I32, F64, F64, C.c_char_p, ptr(_Id)],
            "model_add_row": [U64, ptr(_Term), U64, F64, F64, C.c_char_p, ptr(_Id)],
            "model_add_variables": [U64,ptr(_VariableSpec),U64,ptr(_Id),U64],
            "model_add_rows": [U64,ptr(_RowSpec),U64,ptr(_Id),U64],
            "model_add_rows_sparse": [U64,ptr(_SparseRowBatch),ptr(_Id),U64],
            "model_set_objective": [U64, ptr(_Term), U64, I32, F64],
            "model_set_coefficient": [U64, _Id, _Id, F64],
            "model_set_objective_coefficient": [U64, _Id, F64], "model_set_objective_offset": [U64, F64],
            "solve": [U64, ptr(_Options), ptr(U64)], "session_solve": [U64, U64, ptr(_Options), ptr(U64)],
            "session_create": [ptr(U64)], "session_reset": [U64], "session_statistics": [U64, ptr(_Statistics), U64],
            "model_add_all_different": [U64, ptr(_Id), U64, C.c_char_p, ptr(_Id)],
            "model_add_element": [U64, _Id, ptr(_Id), U64, _Id, I64, C.c_char_p, ptr(_Id)],
            "model_add_table": [U64, ptr(_Id), U64, ptr(I64), U64, U64, C.c_char_p, ptr(_Id)],
            "model_add_cumulative": [U64, ptr(_Id), U64, ptr(I64), U64, ptr(I64), U64, I64, C.c_char_p, ptr(_Id)],
            "model_add_circuit": [U64, ptr(_Id), U64, I64, C.c_char_p, ptr(_Id)],
            "model_add_regular": [U64,ptr(_Id),U64,U64,U64,ptr(_RegularTransition),U64,U64,ptr(U64),U64,C.c_char_p,ptr(_Id)],
            "model_remove_global": [U64, _Id], "model_set_global_name": [U64, _Id, C.c_char_p],
            "model_add_indicator": [U64, _Id, I32, ptr(_Term), U64, F64, F64, C.c_char_p, ptr(_Id), ptr(I32), ptr(_Id)],
            "model_remove_indicator": [U64, _Id],
            "model_add_boolean_and": [U64, _Id, ptr(_Id), U64, C.c_char_p],
            "model_add_boolean_or": [U64, _Id, ptr(_Id), U64, C.c_char_p],
            "cancellation_create": [ptr(U64)], "cancellation_cancel": [U64],
            "cancellation_is_cancelled": [U64,ptr(I32)],
            "cancellation_copy": [U64,ptr(U64)],
            "result_info": [U64, ptr(_Info), U64], "result_number": [U64, I32, ptr(I32), ptr(F64)],
            "result_value": [U64, _Id, ptr(F64)],
            "result_values": [U64, ptr(F64), ptr(U8), ptr(U8), U64, ptr(U64)],
            "result_text": [U64, I32, ptr(C.c_char), U64, ptr(U64)],
            "pool_options_default": [ptr(_PoolOptions),U64], "pool_solve": [U64,ptr(_PoolOptions),ptr(U64)],
            "pool_info": [U64,ptr(_PoolInfo),U64], "pool_projection": [U64,ptr(_Id),U64,ptr(U64)],
            "pool_entry_info": [U64,U64,ptr(_PoolEntryInfo),U64], "pool_entry_result": [U64,U64,ptr(U64)],
            "pool_entry_projection": [U64,U64,ptr(I64),U64,ptr(U64)],
            "pool_attempt_info": [U64,U64,ptr(_PoolAttemptInfo),U64], "pool_message": [U64,ptr(C.c_char),U64,ptr(U64)],
            "repair_options_default": [ptr(_RepairOptions),U64], "repair_solve": [U64,ptr(_RepairOptions),ptr(U64)],
            "repair_info": [U64,ptr(_RepairInfo),U64], "repair_number": [U64,I32,ptr(I32),ptr(F64)],
            "repair_original_value": [U64,_Id,ptr(F64)], "repair_original_values": [U64,ptr(F64),ptr(U8),ptr(U8),U64,ptr(U64)],
            "repair_variable_map": [U64,ptr(_Id),ptr(_Id),ptr(U8),U64,ptr(U64)],
            "repair_validation": [U64,ptr(_ValidationInfo),U64], "repair_item_info": [U64,U64,ptr(_RepairItemInfo),U64],
            "repair_stage_info": [U64,U64,ptr(_RepairStageInfo),U64], "repair_stage_result": [U64,U64,ptr(U64)],
            "repair_final_result": [U64,ptr(U64)], "repair_violation_lock": [U64,ptr(I32),ptr(_Id)],
            "repair_objective_values": [U64,ptr(F64),U64,ptr(U64)],
            "repair_text": [U64,I32,U64,ptr(C.c_char),U64,ptr(U64)],
        }
        for entity in ("model", "session", "result", "cancellation", "pool", "repair", "quadratic_model", "quadratic_result", "lp_observed_result", "basis", "basis_result", "scenario_batch", "lp_evidence", "lp_evidence_stage", "sensitivity"):
            signatures[entity+"_destroy"]=[U64]
        for entity in ("variable", "row"):
            signatures["model_set_"+entity+"_bounds"]=[U64, _Id, F64, F64]
            signatures["model_set_"+entity+"_name"]=[U64, _Id, C.c_char_p]
            signatures["model_remove_"+entity]=[U64, _Id]
        self._functions={}
        for name, signature in signatures.items():
            try:
                function=getattr(self._dll, "gecode_opt_v1_"+name)
            except AttributeError as error:
                raise RuntimeError("C ABI library lacks required additive symbol: gecode_opt_v1_"+name) from error
            function.argtypes=signature; function.restype=I32
            self._functions[name]=function
        # Runtime structure-size handshake catches ctypes layout mismatches.
        temporary=_Options();self.call("options_default", C.byref(temporary), C.sizeof(temporary))

    def call(self, name, *args):
        code=self._functions[name](*args)
        if code:
            message=self._dll.gecode_opt_v1_last_error()
            raise ApiError(code, message.decode("utf-8", errors="replace") if message else "C ABI error")

    def lp_observation_capabilities(self):
        out=_LpCapabilities();self.call("lp_capabilities",C.byref(out),C.sizeof(out))
        def text(field,index=0):
            count=U64();self.call("lp_capability_text",field,index,None,0,C.byref(count))
            buffer=C.create_string_buffer(count.value)
            self.call("lp_capability_text",field,index,buffer,count.value,C.byref(count))
            return buffer.value.decode("utf-8",errors="replace")
        return LpObservationCapabilities(bool(out.available),bool(out.duals),bool(out.basis_export),
            text(0),text(1),tuple(text(2,i) for i in range(out.limitation_count)))

    def quadratic_capabilities(self):
        """Availability of the explicit finite-box continuous numerical QP scope."""
        available=I32();self.call("quadratic_capabilities",C.byref(available))
        return {"available":bool(available.value)}

    def capabilities(self, backend=Backend.AUTO):
        available=I32(); lp=I32(); mip=I32()
        self.call("capabilities", int(Backend(backend)), C.byref(available), C.byref(lp), C.byref(mip))
        return {"available": bool(available.value), "linear_programming": bool(lp.value), "mixed_integer_linear": bool(mip.value)}


_libraries={}
_library_lock=threading.Lock()
def load_library(path=None):
    if isinstance(path, Library):
        return path
    if path is None:
        path=os.environ.get("GECODE_OPTIMIZE_LIBRARY") or bundled_library_path() or find_library("gecodeoptimize_c")
    if not path:
        raise RuntimeError("set GECODE_OPTIMIZE_LIBRARY to the built shared C ABI library")
    key=os.fspath(path)
    with _library_lock:
        if key not in _libraries:
            _libraries[key]=Library(key)
        return _libraries[key]


@dataclass(frozen=True)
class Variable:
    model_id: int
    slot: int
    _library: Library=field(repr=False)
    def _id(self):
        return _Id(_integer(self.model_id,64), _integer(self.slot,64), 1, 0)
@dataclass(frozen=True)
class Row:
    model_id: int
    slot: int
    _library: Library=field(repr=False)
    def _id(self):
        return _Id(_integer(self.model_id,64), _integer(self.slot,64), 2, 0)

@dataclass(frozen=True)
class GlobalConstraint:
    model_id: int
    slot: int
    _library: Library=field(repr=False)
    def _id(self):
        return _Id(_integer(self.model_id,64), _integer(self.slot,64), 3, 0)
@dataclass(frozen=True)
class Indicator:
    model_id: int
    slot: int
    _library: Library=field(repr=False)
    inactive_gate: object=None
    def _id(self):
        return _Id(_integer(self.model_id,64), _integer(self.slot,64), 4, 0)


def _entity(value, library, expected=None):
    if not isinstance(value, expected or (Variable, Row, GlobalConstraint, Indicator)):
        raise TypeError("wrong model entity handle type")
    if value._library is not library:
        raise ValueError("handle belongs to another loaded ABI instance")
    return value._id()


def _terms(items, library):
    if hasattr(items, "items"):
        items=items.items()
    data=[_Term(_entity(v,library,Variable),float(c)) for v,c in items]
    return (_Term*len(data))(*data)


def _ids(values, library):
    data=[_entity(value,library,Variable) for value in values]
    return (_Id*len(data))(*data)


def _int64s(values):
    data=[_integer(value,64,True) for value in values]
    return (I64*len(data))(*data)


@dataclass
class VariableSpec:
    type: VariableType=VariableType.CONTINUOUS
    lower: float=0.0
    upper: object=None  # As with scalar construction: Binary 1, otherwise +inf.
    name: str=""


@dataclass
class RowSpec:
    terms: object=()
    lower: float=-math.inf
    upper: float=math.inf
    name: str=""


@dataclass
class SparseRowBatch:
    columns: object=()
    row_start: object=(0,)
    column: object=()
    coefficient: object=()
    lower: object=()
    upper: object=()
    names: object=()


@dataclass
class Options:
    backend: Backend=Backend.AUTO
    guarantee: Guarantee=Guarantee.NUMERICAL
    threads: int=1
    random_seed: int=0
    time_limit_seconds: float=math.inf
    relative_gap: float=1e-4
    absolute_gap: float=1e-6
    feasibility_tolerance: float=1e-7
    integrality_tolerance: float=1e-6
    node_limit: object=None
    cancellation: object=None
    primal_start: object=()

    def _marshal(self, library):
        out=_Options();library.call("options_default",C.byref(out),C.sizeof(out))
        out.backend=int(Backend(self.backend));out.guarantee=int(Guarantee(self.guarantee))
        out.threads=_integer(self.threads,32,True);out.random_seed=_integer(self.random_seed,32,True)
        for name in ("time_limit_seconds","relative_gap","absolute_gap","feasibility_tolerance","integrality_tolerance"):
            setattr(out,name,float(getattr(self,name)))
        if self.node_limit is not None:
            out.has_node_limit=1;out.node_limit=_integer(self.node_limit,64)
        if self.cancellation is not None:
            if not isinstance(self.cancellation,Cancellation) or self.cancellation._library is not library:
                raise TypeError("cancellation must belong to the same ABI instance")
            out.cancellation=self.cancellation._open()
        entries=self.primal_start.items() if hasattr(self.primal_start,"items") else self.primal_start
        starts=[_Start(_entity(v,library,Variable),float(value)) for v,value in entries]
        array=(_Start*len(starts))(*starts)
        out.primal_start=array;out.primal_start_count=len(starts)
        out._keepalive=(array,self.cancellation)
        return out


@dataclass
class PoolOptions:
    solve: Options=field(default_factory=Options)
    max_solutions: int=10
    projection: object=None
    def _marshal(self, library):
        if not isinstance(self.solve,Options):
            raise TypeError("solve must be Options")
        out=_PoolOptions();library.call("pool_options_default",C.byref(out),C.sizeof(out))
        solve=self.solve._marshal(library);out.solve=solve
        out.max_solutions=_integer(self.max_solutions,64)
        array=None
        if self.projection is not None:
            array=_ids(self.projection,library);out.has_projection=1
            out.projection=array;out.projection_count=len(array)
        out._keepalive=(solve,array)
        return out


@dataclass(frozen=True)
class RelaxationSelection:
    source: object
    side: RelaxationSide=RelaxationSide.LOWER
    penalty: float=1.0


@dataclass
class RepairOptions:
    solve: Options=field(default_factory=Options)
    selections: object=()
    optimize_original_objective: bool=False
    def _marshal(self, library):
        if not isinstance(self.solve,Options):
            raise TypeError("solve must be Options")
        if type(self.optimize_original_objective) is not bool:
            raise TypeError("optimize_original_objective must be bool")
        data=[]
        for selection in self.selections:
            if not isinstance(selection,RelaxationSelection):
                raise TypeError("selections must contain RelaxationSelection records")
            data.append(_Selection(_entity(selection.source,library,(Variable,Row)),
                        int(RelaxationSide(selection.side)),0,float(selection.penalty)))
        out=_RepairOptions();library.call("repair_options_default",C.byref(out),C.sizeof(out))
        solve=self.solve._marshal(library);out.solve=solve
        array=(_Selection*len(data))(*data);out.selections=array;out.selection_count=len(array)
        out.optimize_original_objective=int(self.optimize_original_objective)
        out._keepalive=(solve,array)
        return out


class _Owner:
    _kind=None
    def _initialize(self, library, handle=None):
        self._library=load_library(library);self._handle=0
        if handle is None:
            out=U64();self._library.call(self._kind+"_create",C.byref(out));self._handle=out.value
        else:
            self._handle=handle
    @property
    def closed(self):
        return not self._handle
    def _open(self):
        if self.closed:
            raise RuntimeError(self._kind+" is closed")
        return self._handle
    def close(self):
        if getattr(self,"_handle",0):
            self._library.call(self._kind+"_destroy",self._handle)
            self._handle=0
    def __enter__(self):
        self._open();return self
    def __exit__(self,*unused):
        self.close()
    def __del__(self):
        try:
            self.close()
        except Exception:
            pass


class Model(_Owner):
    _kind="model"
    def __init__(self, library=None):
        self._initialize(library)
    @classmethod
    def read(cls, filename, library=None):
        """Read LP/MPS into an independent owner, closed explicitly or by context."""
        lib = load_library(library)
        model = cls.__new__(cls)
        model._initialize(lib, 0)
        out = U64()
        lib.call("model_read", _text(os.fspath(filename)), C.byref(out))
        try:
            model._handle = out.value
        except BaseException:
            lib.call("model_destroy", out)
            raise
        return model
    @property
    def identity(self):
        owner=U64();revision=U64();self._library.call("model_identity",self._open(),C.byref(owner),C.byref(revision))
        return owner.value,revision.value
    def add_variable(self, type=VariableType.CONTINUOUS, lower=0.0, upper=None, name=""):
        kind=VariableType(type);upper=(1.0 if kind==VariableType.BINARY else math.inf) if upper is None else upper
        out=_Id();self._library.call("model_add_variable",self._open(),int(kind),float(lower),float(upper),_text(name),C.byref(out))
        return Variable(out.model_id,out.slot,self._library)
    def add_row(self, terms=(), lower=-math.inf, upper=math.inf, name=""):
        array=_terms(terms,self._library);out=_Id()
        self._library.call("model_add_row",self._open(),array,len(array),float(lower),float(upper),_text(name),C.byref(out))
        return Row(out.model_id,out.slot,self._library)
    def add_variables(self, specs):
        """Atomically add an ordinary sequence of VariableSpec records."""
        data=[];keepalive=[]
        for spec in specs:
            if not isinstance(spec,VariableSpec):
                raise TypeError("expected VariableSpec entries")
            kind=VariableType(spec.type)
            upper=(1.0 if kind==VariableType.BINARY else math.inf) if spec.upper is None else spec.upper
            name=_text(spec.name);keepalive.append(name)
            data.append(_VariableSpec(C.sizeof(_VariableSpec),int(kind),0,float(spec.lower),float(upper),name))
        array=(_VariableSpec*len(data))(*data);out=(_Id*len(data))()
        self._library.call("model_add_variables",self._open(),array,len(array),out,len(out))
        return tuple(Variable(v.model_id,v.slot,self._library) for v in out)
    def add_rows(self, specs):
        """Atomically add RowSpec entries; each terms field accepts a mapping or pairs."""
        data=[];keepalive=[]
        for spec in specs:
            if not isinstance(spec,RowSpec):
                raise TypeError("expected RowSpec entries")
            terms=_terms(spec.terms,self._library);name=_text(spec.name);keepalive.append((terms,name))
            data.append(_RowSpec(C.sizeof(_RowSpec),0,terms,len(terms),float(spec.lower),float(spec.upper),name))
        array=(_RowSpec*len(data))(*data);out=(_Id*len(data))()
        self._library.call("model_add_rows",self._open(),array,len(array),out,len(out))
        return tuple(Row(r.model_id,r.slot,self._library) for r in out)
    def add_rows_sparse(self, batch):
        """Add a CSR batch in one C call, copying ordinary sequences, without NumPy."""
        if not isinstance(batch,SparseRowBatch):
            raise TypeError("expected SparseRowBatch")
        record=_SparseRowBatch();record.struct_size=C.sizeof(record);keepalive=[]
        columns=_ids(batch.columns,self._library);record.columns=columns;record.columns_count=len(columns)
        keepalive.append(columns)
        for name in ("row_start","column"):
            data=[_integer(v,64) for v in getattr(batch,name)]
            array=(U64*len(data))(*data);setattr(record,name,array);setattr(record,name+"_count",len(array))
            keepalive.append(array)
        for name in ("coefficient","lower","upper"):
            data=[float(v) for v in getattr(batch,name)]
            array=(F64*len(data))(*data);setattr(record,name,array);setattr(record,name+"_count",len(array))
            keepalive.append(array)
        names=[_text(v) for v in batch.names];array=(C.c_char_p*len(names))(*names)
        record.names=array;record.names_count=len(array);keepalive.extend((names,array))
        out=(_Id*record.lower_count)()
        self._library.call("model_add_rows_sparse",self._open(),C.byref(record),out,len(out))
        return tuple(Row(r.model_id,r.slot,self._library) for r in out)
    def set_objective(self, terms=(), *, maximize=False, offset=0.0):
        if type(maximize) is not bool:
            raise TypeError("maximize must be bool")
        array=_terms(terms,self._library);self._library.call("model_set_objective",self._open(),array,len(array),int(maximize),float(offset))
    def set_bounds(self, entity, lower, upper):
        identifier=_entity(entity,self._library,(Variable,Row));kind="variable" if isinstance(entity,Variable) else "row"
        self._library.call("model_set_"+kind+"_bounds",self._open(),identifier,float(lower),float(upper))
    def set_coefficient(self, row, variable, value):
        self._library.call("model_set_coefficient",self._open(),_entity(row,self._library,Row),_entity(variable,self._library,Variable),float(value))
    def set_objective_coefficient(self, variable, value):
        self._library.call("model_set_objective_coefficient",self._open(),_entity(variable,self._library,Variable),float(value))
    def set_objective_offset(self, value):
        self._library.call("model_set_objective_offset",self._open(),float(value))
    def set_name(self, entity, name):
        identifier=_entity(entity,self._library,(Variable,Row,GlobalConstraint))
        kind="variable" if isinstance(entity,Variable) else "row" if isinstance(entity,Row) else "global"
        self._library.call("model_set_"+kind+"_name",self._open(),identifier,_text(name))
    def remove(self, entity):
        identifier=_entity(entity,self._library)
        kind=("variable" if isinstance(entity,Variable) else "row" if isinstance(entity,Row)
              else "global" if isinstance(entity,GlobalConstraint) else "indicator")
        self._library.call("model_remove_"+kind,self._open(),identifier)
    def add_all_different(self, variables, name=""):
        array=_ids(variables,self._library);out=_Id()
        self._library.call("model_add_all_different",self._open(),array,len(array),_text(name),C.byref(out))
        return GlobalConstraint(out.model_id,out.slot,self._library)
    def add_element(self, index, elements, result, index_base=0, name=""):
        array=_ids(elements,self._library);out=_Id()
        self._library.call("model_add_element",self._open(),_entity(index,self._library,Variable),array,len(array),
                           _entity(result,self._library,Variable),_integer(index_base,64,True),_text(name),C.byref(out))
        return GlobalConstraint(out.model_id,out.slot,self._library)
    def add_table(self, variables, tuples, name=""):
        array=_ids(variables,self._library);rows=[tuple(row) for row in tuples]
        if any(len(row)!=len(array) for row in rows):
            raise ValueError("table tuple arity must match its variable count")
        values=_int64s(value for row in rows for value in row);out=_Id()
        self._library.call("model_add_table",self._open(),array,len(array),values,len(values),len(rows),_text(name),C.byref(out))
        return GlobalConstraint(out.model_id,out.slot,self._library)
    def add_cumulative(self, starts, durations, heights, capacity, name=""):
        array=_ids(starts,self._library);ds=_int64s(durations);hs=_int64s(heights);out=_Id()
        self._library.call("model_add_cumulative",self._open(),array,len(array),ds,len(ds),hs,len(hs),
                           _integer(capacity,64,True),_text(name),C.byref(out))
        return GlobalConstraint(out.model_id,out.slot,self._library)
    def add_circuit(self, successors, index_base=0, name=""):
        array=_ids(successors,self._library);out=_Id()
        self._library.call("model_add_circuit",self._open(),array,len(array),_integer(index_base,64,True),_text(name),C.byref(out))
        return GlobalConstraint(out.model_id,out.slot,self._library)
    def add_regular(self, variables, state_count, initial_state, transitions, final_states, name=""):
        array=_ids(variables,self._library)
        edges=[]
        for edge in transitions:
            if not isinstance(edge,RegularTransition):
                raise TypeError("transitions must contain RegularTransition records")
            edges.append(_RegularTransition(C.sizeof(_RegularTransition),0,_integer(edge.from_state,64),
                _integer(edge.symbol,64,True),_integer(edge.to_state,64)))
        edges=(_RegularTransition*len(edges))(*edges)
        finals=[_integer(value,64) for value in final_states];finals=(U64*len(finals))(*finals);out=_Id()
        self._library.call("model_add_regular",self._open(),array,len(array),_integer(state_count,64),_integer(initial_state,64),
            edges,len(edges),C.sizeof(_RegularTransition),finals,len(finals),_text(name),C.byref(out))
        return GlobalConstraint(out.model_id,out.slot,self._library)
    def add_indicator(self, activator, active_value, terms=(), lower=-math.inf, upper=math.inf, name=""):
        if type(active_value) is not bool:
            raise TypeError("indicator active_value must be bool")
        array=_terms(terms,self._library);out=_Id();has_gate=I32();gate=_Id()
        self._library.call("model_add_indicator",self._open(),_entity(activator,self._library,Variable),int(active_value),
                           array,len(array),float(lower),float(upper),_text(name),C.byref(out),C.byref(has_gate),C.byref(gate))
        auxiliary=Variable(gate.model_id,gate.slot,self._library) if has_gate.value else None
        return Indicator(out.model_id,out.slot,self._library,auxiliary)
    def add_boolean_and(self, result, inputs=(), name=""):
        array=_ids(inputs,self._library)
        self._library.call("model_add_boolean_and",self._open(),_entity(result,self._library,Variable),array,len(array),_text(name))
    def add_boolean_or(self, result, inputs=(), name=""):
        array=_ids(inputs,self._library)
        self._library.call("model_add_boolean_or",self._open(),_entity(result,self._library,Variable),array,len(array),_text(name))
    def write(self, filename):
        self._library.call("model_write",self._open(),_text(os.fspath(filename)))
    def solve(self, options=None):
        if options is not None and not isinstance(options,Options):
            raise TypeError("options must be Options")
        native=(options or Options())._marshal(self._library);out=U64()
        self._library.call("solve",self._open(),C.byref(native),C.byref(out));return Result(self._library,out.value)
    def analyze_lp_evidence(self, options=None):
        """Explicit numerical ray/Farkas recovery in private auxiliary models."""
        started=time.monotonic()
        if options is not None and not isinstance(options,LpEvidenceOptions):
            raise TypeError("options must be LpEvidenceOptions")
        native=(options or LpEvidenceOptions())._marshal(self._library)
        if math.isfinite(native.solve.time_limit_seconds) and native.solve.time_limit_seconds>=0:
            native.solve.time_limit_seconds=max(0.0,native.solve.time_limit_seconds-(time.monotonic()-started))
        return _own_call(self,LpEvidenceResult,"analyze_lp_evidence",C.byref(native))
    def solve_scenarios(self, definitions, options=None):
        """Serial owning batch. Sparse absolute patches; one whole-batch allowance."""
        start=time.monotonic()
        if options is not None and not isinstance(options,ScenarioOptions):
            raise TypeError("options must be ScenarioOptions")
        native=(options or ScenarioOptions())._marshal(self._library)
        array=_scenario_definitions(definitions,self._library)
        if math.isfinite(native.solve.time_limit_seconds) and native.solve.time_limit_seconds>=0:
            native.solve.time_limit_seconds=max(0.0,native.solve.time_limit_seconds-(time.monotonic()-start))
        return _own_call(self,ScenarioBatchResult,"solve_scenarios",array,len(array),C.sizeof(_ScenarioDefinition),C.byref(native))
    def solve_lp_observed(self, options=None):
        if options is not None and not isinstance(options,LpObservationOptions):
            raise TypeError("options must be LpObservationOptions")
        native=(options or LpObservationOptions())._marshal(self._library)
        return _own_call(self,LpObservedResult,"solve_lp_observed",C.byref(native))
    def solve_lp_with_basis(self, basis, options=None):
        native=_basis_solve_options(self,basis,options)
        return _own_call(self,LpBasisSolveResult,"solve_lp_with_basis",basis._open(),C.byref(native))
    def solve_pool(self, options=None):
        if options is not None and not isinstance(options,PoolOptions):
            raise TypeError("options must be PoolOptions")
        native=(options or PoolOptions())._marshal(self._library)
        return _own_call(self,PoolResult,"pool_solve",C.byref(native))
    def relax_feasibility(self, options=None):
        if options is not None and not isinstance(options,RepairOptions):
            raise TypeError("options must be RepairOptions")
        native=(options or RepairOptions())._marshal(self._library)
        return _own_call(self,RepairResult,"repair_solve",C.byref(native))


class Cancellation(_Owner):
    _kind="cancellation"
    def __init__(self, library=None):
        self._initialize(library)
    def cancel(self):
        self._library.call("cancellation_cancel",self._open())
    @property
    def cancelled(self):
        out=I32();self._library.call("cancellation_is_cancelled",self._open(),C.byref(out));return bool(out.value)
    def copy(self):
        """Independent owner of the same thread-safe cancellation state."""
        return _own_call(self,Cancellation,"cancellation_copy")


class Session(_Owner):
    _kind="session"
    def __init__(self, library=None):
        self._initialize(library)
    def reset(self):
        self._library.call("session_reset",self._open())
    @property
    def statistics(self):
        out=_Statistics();self._library.call("session_statistics",self._open(),C.byref(out),C.sizeof(out))
        return {name:getattr(out,name) for name,_ in out._fields_}
    def solve(self, model, options=None):
        if not isinstance(model,Model) or model._library is not self._library:
            raise TypeError("model must belong to the same ABI instance")
        if options is not None and not isinstance(options,Options):
            raise TypeError("options must be Options")
        native=(options or Options())._marshal(self._library);out=U64()
        self._library.call("session_solve",self._open(),model._open(),C.byref(native),C.byref(out));return Result(self._library,out.value)
    def solve_lp_observed(self, model, options=None):
        if not isinstance(model,Model) or model._library is not self._library:
            raise TypeError("model must belong to the same ABI instance")
        if options is not None and not isinstance(options,LpObservationOptions):
            raise TypeError("options must be LpObservationOptions")
        native=(options or LpObservationOptions())._marshal(self._library)
        return _own_call(self,LpObservedResult,"session_solve_lp_observed",model._open(),C.byref(native))

    def solve_lp_with_basis(self, model, basis, options=None):
        if not isinstance(model,Model) or model._library is not self._library:
            raise TypeError("model must belong to the same ABI instance")
        native=_basis_solve_options(self,basis,options)
        return _own_call(self,LpBasisSolveResult,"session_solve_lp_with_basis",model._open(),basis._open(),C.byref(native))


class Result(_Owner):
    """Immutable historical result; remains usable after Model/Session.close()."""
    _kind="result"
    def __init__(self, library, handle):
        self._initialize(library,handle)
    @property
    def info(self):
        out=_Info();self._library.call("result_info",self._open(),C.byref(out),C.sizeof(out))
        data={name:getattr(out,name) for name,_ in out._fields_ if name!="reserved"}
        data["termination"]=Termination(data["termination"]);data["guarantee"]=Guarantee(data["guarantee"])
        for key in ("has_solution","solution_validated","start_submitted"):
            data[key]=bool(data[key])
        return data
    @property
    def termination(self):
        return self.info["termination"]
    @property
    def has_solution(self):
        return self.info["has_solution"]
    def _number(self, field):
        present=I32();value=F64();self._library.call("result_number",self._open(),field,C.byref(present),C.byref(value))
        return value.value if present.value else None
    objective=property(lambda self:self._number(0))
    best_bound=property(lambda self:self._number(1))
    absolute_gap=property(lambda self:self._number(2))
    relative_gap=property(lambda self:self._number(3))
    native_gap=property(lambda self:self._number(4))
    def value(self, variable):
        out=F64();self._library.call("result_value",self._open(),_entity(variable,self._library,Variable),C.byref(out));return out.value
    @property
    def values(self):
        """Copy per-slot records: active, present, and value (None if absent)."""
        count=U64();self._library.call("result_values",self._open(),None,None,None,0,C.byref(count))
        values=(F64*count.value)();active=(U8*count.value)();present=(U8*count.value)()
        self._library.call("result_values",self._open(),values,active,present,count.value,C.byref(count))
        return [{"active":bool(active[i]),"present":bool(present[i]),"value":values[i] if present[i] else None} for i in range(count.value)]
    def _text(self, field):
        size=U64();self._library.call("result_text",self._open(),field,None,0,C.byref(size))
        buffer=C.create_string_buffer(size.value);self._library.call("result_text",self._open(),field,buffer,size.value,C.byref(size))
        return buffer.value.decode("utf-8",errors="replace")
    backend=property(lambda self:self._text(0))
    backend_version=property(lambda self:self._text(1))
    message=property(lambda self:self._text(2))


def _own_call(owner, cls, function, *args):
    """Prepare the Python owner before allocating its C token."""
    result=cls.__new__(cls);result._initialize(owner._library,0)
    out=U64();owner._library.call(function,owner._open(),*args,C.byref(out))
    try:
        result._handle=out.value
    except BaseException:
        owner._library.call(cls._kind+"_destroy",out)
        raise
    return result


def _record(owner, function, cls, *args):
    out=cls();owner._library.call(function,owner._open(),*args,C.byref(out),C.sizeof(out));return out


def _metadata(record):
    result={}
    for name,_ in record._fields_:
        if name!="reserved":
            value=getattr(record,name)
            result[name]=(value.value if value.present else None) if isinstance(value,_OptionalNumber) else value
    return result


def _array(owner, function, element, *args):
    count=U64();owner._library.call(function,owner._open(),*args,None,0,C.byref(count))
    buffer=(element*count.value)();owner._library.call(function,owner._open(),*args,buffer,count.value,C.byref(count))
    return buffer


def _copied_text(owner, function, *args):
    buffer=_array(owner,function,C.c_char,*args)
    return buffer.value.decode("utf-8",errors="replace")


def _returned_id(identifier, library):
    if identifier.model_id==0:
        return None
    classes={1:Variable,2:Row,3:GlobalConstraint,4:Indicator}
    if identifier.reserved or identifier.kind not in classes:
        raise RuntimeError("C ABI returned an invalid entity identity")
    return classes[identifier.kind](identifier.model_id,identifier.slot,library)


@dataclass(frozen=True)
class PoolEntry:
    projection_values: tuple
    rank_established: bool


@dataclass(frozen=True)
class PoolAttempt:
    termination: Termination
    guarantee: Guarantee
    candidate_accepted: bool
    rank_established: bool
    objective: object
    remaining_bound: object


class PoolResult(_Owner):
    """Owning workflow; use entry_result(i) to obtain an independent Result."""
    _kind="pool"
    def __init__(self, library, handle):
        self._initialize(library,handle)
    @property
    def info(self):
        data=_metadata(_record(self,"pool_info",_PoolInfo))
        data["termination"]=Termination(data["termination"]);data["guarantee"]=Guarantee(data["guarantee"])
        data["completion"]=PoolCompletion(data["completion"]);return data
    termination=property(lambda self:self.info["termination"])
    completion=property(lambda self:self.info["completion"])
    exhausted=property(lambda self:self.completion==PoolCompletion.EXHAUSTED)
    ranked_prefix=property(lambda self:self.info["ranked_prefix"])
    message=property(lambda self:_copied_text(self,"pool_message"))
    @property
    def projection(self):
        return tuple(_returned_id(value,self._library) for value in _array(self,"pool_projection",_Id))
    def entry(self, index):
        index=_integer(index,64);info=_record(self,"pool_entry_info",_PoolEntryInfo,index)
        return PoolEntry(tuple(_array(self,"pool_entry_projection",I64,index)),bool(info.rank_established))
    def entry_result(self, index):
        return _own_call(self,Result,"pool_entry_result",_integer(index,64))
    def attempt(self, index):
        info=_metadata(_record(self,"pool_attempt_info",_PoolAttemptInfo,_integer(index,64)))
        info["termination"]=Termination(info["termination"]);info["guarantee"]=Guarantee(info["guarantee"])
        info["candidate_accepted"]=bool(info["candidate_accepted"]);info["rank_established"]=bool(info["rank_established"])
        return PoolAttempt(**info)


@dataclass(frozen=True)
class RepairItem:
    source: object
    slack: object
    penalty_row: object
    side: RelaxationSide
    original_bound: float
    penalty: float
    activity: object
    violation: object
    weighted_violation: object
    slack_value: object
    name: str


@dataclass(frozen=True)
class RepairStage:
    index: int
    completed: bool
    retention_bound: object
    name: str


@dataclass(frozen=True)
class RepairVariable:
    source: Variable
    private: object
    active: bool


@dataclass(frozen=True)
class Validation:
    valid: bool
    model_valid: bool
    violated_globals: int
    max_bound_violation: float
    max_row_violation: float
    max_integrality_violation: float
    max_indicator_violation: float
    objective: object
    message: str


class RepairResult(_Owner):
    """Owning repair evidence; original values are not a feasible solve result."""
    _kind="repair"
    def __init__(self, library, handle):
        self._initialize(library,handle)
    @property
    def info(self):
        data=_metadata(_record(self,"repair_info",_RepairInfo))
        for name in ("termination","workflow_termination"):
            data[name]=Termination(data[name])
        for name in ("guarantee","workflow_guarantee"):
            data[name]=Guarantee(data[name])
        for name in ("has_private_model","has_repair","minimum_violation_established","original_objective_optimized"):
            data[name]=bool(data[name])
        return data
    termination=property(lambda self:self.info["termination"])
    has_repair=property(lambda self:self.info["has_repair"])
    def _number(self, field):
        present=I32();value=F64();self._library.call("repair_number",self._open(),field,C.byref(present),C.byref(value))
        return value.value if present.value else None
    minimum_weighted_violation=property(lambda self:self._number(0))
    weighted_violation=property(lambda self:self._number(1))
    original_objective=property(lambda self:self._number(2))
    message=property(lambda self:_copied_text(self,"repair_text",0,0))
    workflow_message=property(lambda self:_copied_text(self,"repair_text",1,0))
    def original_value(self, variable):
        out=F64();self._library.call("repair_original_value",self._open(),_entity(variable,self._library,Variable),C.byref(out))
        return out.value
    @property
    def original_values(self):
        count=U64();self._library.call("repair_original_values",self._open(),None,None,None,0,C.byref(count))
        values=(F64*count.value)();active=(U8*count.value)();present=(U8*count.value)()
        self._library.call("repair_original_values",self._open(),values,active,present,count.value,C.byref(count))
        return tuple({"active":bool(active[i]),"present":bool(present[i]),"value":values[i] if present[i] else None} for i in range(count.value))
    @property
    def variable_map(self):
        count=U64();self._library.call("repair_variable_map",self._open(),None,None,None,0,C.byref(count))
        source=(_Id*count.value)();private=(_Id*count.value)();active=(U8*count.value)()
        self._library.call("repair_variable_map",self._open(),source,private,active,count.value,C.byref(count))
        return tuple(RepairVariable(_returned_id(source[i],self._library),_returned_id(private[i],self._library),bool(active[i])) for i in range(count.value))
    @property
    def original_validation(self):
        info=_metadata(_record(self,"repair_validation",_ValidationInfo))
        info["valid"]=bool(info["valid"]);info["model_valid"]=bool(info["model_valid"])
        return Validation(**info,message=_copied_text(self,"repair_text",2,0))
    def item(self, index):
        index=_integer(index,64);info=_metadata(_record(self,"repair_item_info",_RepairItemInfo,index))
        for name in ("source","slack","penalty_row"):
            info[name]=_returned_id(info[name],self._library)
        info["side"]=RelaxationSide(info["side"])
        return RepairItem(**info,name=_copied_text(self,"repair_text",3,index))
    def stage(self, index):
        index=_integer(index,64);info=_metadata(_record(self,"repair_stage_info",_RepairStageInfo,index))
        info["completed"]=bool(info["completed"])
        return RepairStage(**info,name=_copied_text(self,"repair_text",4,index))
    def stage_result(self, index):
        return _own_call(self,Result,"repair_stage_result",_integer(index,64))
    def final_result(self):
        return _own_call(self,Result,"repair_final_result")
    @property
    def violation_lock(self):
        present=I32();out=_Id();self._library.call("repair_violation_lock",self._open(),C.byref(present),C.byref(out))
        return _returned_id(out,self._library) if present.value else None
    objective_values=property(lambda self:tuple(_array(self,"repair_objective_values",F64)))


@dataclass
class WeightedSquare:
    """Positive weight times (sum(coefficient*variable) + offset)**2."""
    terms: object=()
    offset: float=0.0
    weight: float=1.0
    name: str=""


@dataclass
class QuadraticOptions:
    solve: Options=field(default_factory=Options)
    iteration_limit: int=100000
    max_auxiliary_variables: int=100000
    max_lifted_nonzeros: int=2000000
    stationarity_tolerance: float=1e-7
    complementarity_tolerance: float=1e-7
    optimality_tolerance: float=1e-6

    def _marshal(self, library):
        if not isinstance(self.solve,Options):
            raise TypeError("solve must be Options")
        out=_QuadraticOptions();library.call("quadratic_options_default",C.byref(out),C.sizeof(out))
        solve=self.solve._marshal(library);out.solve=solve
        for name in ("iteration_limit","max_auxiliary_variables","max_lifted_nonzeros"):
            setattr(out,name,_integer(getattr(self,name),64))
        for name in ("stationarity_tolerance","complementarity_tolerance","optimality_tolerance"):
            setattr(out,name,float(getattr(self,name)))
        out._keepalive=(solve,)
        return out


class QuadraticModel(_Owner):
    """Distinct finite-box continuous QP owner; no linear Model conversion."""
    _kind="quadratic_model"
    def __init__(self, library=None):
        self._initialize(library)
    @property
    def identity(self):
        owner=U64();revision=U64()
        self._library.call("quadratic_model_identity",self._open(),C.byref(owner),C.byref(revision))
        return owner.value,revision.value
    def add_continuous(self, lower, upper, name=""):
        out=_Id();self._library.call("quadratic_model_add_continuous",self._open(),float(lower),float(upper),_text(name),C.byref(out))
        return Variable(out.model_id,out.slot,self._library)
    def add_row(self, terms=(), lower=-math.inf, upper=math.inf, name=""):
        array=_terms(terms,self._library);out=_Id()
        self._library.call("quadratic_model_add_row",self._open(),array,len(array),float(lower),float(upper),_text(name),C.byref(out))
        return Row(out.model_id,out.slot,self._library)
    def _objective(self, squares, linear, offset, maximize):
        data=[];keepalive=[]
        for square in squares:
            if not isinstance(square,WeightedSquare):
                raise TypeError("squares must contain WeightedSquare records")
            terms=_terms(square.terms,self._library);name=_text(square.name);keepalive.append((terms,name))
            data.append(_WeightedSquare(C.sizeof(_WeightedSquare),0,terms,len(terms),float(square.offset),float(square.weight),name))
        array=(_WeightedSquare*len(data))(*data);linear=_terms(linear,self._library)
        self._library.call("quadratic_model_set_objective",self._open(),array,len(array),linear,len(linear),maximize,float(offset))
    def minimize_squares(self, squares=(), linear=(), offset=0.0):
        """Minimize linear + offset + sum(positive weighted squares)."""
        self._objective(squares,linear,offset,0)
    def maximize_concave_squares(self, squares=(), linear=(), offset=0.0):
        """Maximize linear + offset MINUS sum(positive weighted squares)."""
        self._objective(squares,linear,offset,1)
    def set_bounds(self, entity, lower, upper):
        identifier=_entity(entity,self._library,(Variable,Row))
        kind="variable" if isinstance(entity,Variable) else "row"
        self._library.call("quadratic_model_set_"+kind+"_bounds",self._open(),identifier,float(lower),float(upper))
    def set_coefficient(self, row, variable, value):
        self._library.call("quadratic_model_set_coefficient",self._open(),_entity(row,self._library,Row),
                           _entity(variable,self._library,Variable),float(value))
    def remove(self, entity):
        identifier=_entity(entity,self._library,(Variable,Row))
        kind="variable" if isinstance(entity,Variable) else "row"
        self._library.call("quadratic_model_remove_"+kind,self._open(),identifier)
    def solve(self, options=None):
        if options is not None and not isinstance(options,QuadraticOptions):
            raise TypeError("options must be QuadraticOptions")
        native=(options or QuadraticOptions())._marshal(self._library)
        return _own_call(self,QuadraticResult,"quadratic_solve",C.byref(native))


@dataclass(frozen=True)
class QuadraticChecks:
    primal_valid: bool
    objective_valid: bool
    kkt_available: bool
    kkt_valid: bool
    bound_valid: bool
    max_stationarity: object
    max_complementarity: object
    original_objective: object
    normalized_lower_bound: object
    gap_upper_bound: object
    square_count: int
    gradient_slots: int
    message: str


class QuadraticResult(_Owner):
    """Owning original QP result and numerical checks, distinct from Result."""
    _kind="quadratic_result"
    def __init__(self, library, handle):
        self._initialize(library,handle)
    @property
    def info(self):
        out=_record(self,"quadratic_result_info",_QuadraticInfo);data=_metadata(out.result)
        data["termination"]=Termination(data["termination"]);data["guarantee"]=Guarantee(data["guarantee"])
        for name in ("has_solution","solution_validated","start_submitted"):
            data[name]=bool(data[name])
        data["qp_iterations"]=out.qp_iterations;data["regularization"]=out.regularization
        return data
    termination=property(lambda self:self.info["termination"])
    has_solution=property(lambda self:self.info["has_solution"])
    def _number(self, field):
        present=I32();value=F64()
        self._library.call("quadratic_result_number",self._open(),field,C.byref(present),C.byref(value))
        return value.value if present.value else None
    objective=property(lambda self:self._number(0))
    best_bound=property(lambda self:self._number(1))
    absolute_gap=property(lambda self:self._number(2))
    relative_gap=property(lambda self:self._number(3))
    native_gap=property(lambda self:self._number(4))
    vendor_objective=property(lambda self:self._number(5))
    vendor_dual_estimate=property(lambda self:self._number(6))
    def value(self, variable):
        out=F64();self._library.call("quadratic_result_value",self._open(),_entity(variable,self._library,Variable),C.byref(out))
        return out.value
    @property
    def values(self):
        count=U64();self._library.call("quadratic_result_values",self._open(),None,None,None,0,C.byref(count))
        values=(F64*count.value)();active=(U8*count.value)();present=(U8*count.value)()
        self._library.call("quadratic_result_values",self._open(),values,active,present,count.value,C.byref(count))
        return tuple({"active":bool(active[i]),"present":bool(present[i]),"value":values[i] if present[i] else None}
                     for i in range(count.value))
    backend=property(lambda self:_copied_text(self,"quadratic_result_text",0))
    backend_version=property(lambda self:_copied_text(self,"quadratic_result_text",1))
    message=property(lambda self:_copied_text(self,"quadratic_result_text",2))
    @property
    def checks(self):
        data=_metadata(_record(self,"quadratic_result_checks",_QuadraticChecks))
        for name in ("primal_valid","objective_valid","kkt_available","kkt_valid","bound_valid"):
            data[name]=bool(data[name])
        return QuadraticChecks(**data,message=_copied_text(self,"quadratic_result_text",3))
    @property
    def square_residuals(self):
        """Original affine residuals, not their squared/weighted contributions."""
        if not self.checks.objective_valid:
            return None
        return tuple(_array(self,"quadratic_result_array",F64,0))
    @property
    def original_gradient(self):
        if not self.checks.objective_valid:
            return None
        return tuple(_array(self,"quadratic_result_array",F64,1))


class LpObservationState(IntEnum):
    NOT_REQUESTED=0; AVAILABLE=1; UNAVAILABLE=2; REJECTED=3
class LpObservationReason(IntEnum):
    NONE=0; NOT_REQUESTED=1; UNSUPPORTED=2; NO_BACKEND_SOLVE=3; NO_PRIMAL_POINT=4
    NOT_OPTIMAL=5; NO_DUAL_POINT=6; NO_BASIS=7; ELIDED_CONSTANT_ROWS=8; INTERRUPTED=9
    INVALID_BACKEND_DATA=10; FAILED_CHECKS=11; ALLOCATION_FAILURE=12; INVALID_MODEL=13
class LpBasisStatus(IntEnum):
    LOWER=0; BASIC=1; UPPER=2; ZERO=3; NONBASIC_UNSPECIFIED=4
class LpDualSource(IntEnum):
    NONE=0; BACKEND=1; DERIVED_CONSTANT_ROW=2


@dataclass
class LpObservationOptions:
    solve: Options=field(default_factory=Options)
    duals: bool=True
    basis: bool=True
    dual_feasibility: float=1e-7
    stationarity: float=1e-7
    complementarity: float=1e-6
    objective_gap: float=1e-6
    def _marshal(self, library):
        if not isinstance(self.solve,Options):
            raise TypeError("solve must be Options")
        if type(self.duals) is not bool or type(self.basis) is not bool:
            raise TypeError("duals and basis must be bool")
        out=_LpOptions();library.call("lp_options_default",C.byref(out),C.sizeof(out))
        solve=self.solve._marshal(library);out.solve=solve
        out.duals=self.duals;out.basis=self.basis
        for name in ("dual_feasibility","stationarity","complementarity","objective_gap"):
            setattr(out,name,float(getattr(self,name)))
        out._keepalive=(solve,)
        return out


@dataclass(frozen=True)
class LpObservationCapabilities:
    available: bool
    duals: bool
    basis_export: bool
    backend: str
    backend_version: str
    limitations: tuple
@dataclass(frozen=True)
class LpObservationGroup:
    state: LpObservationState
    reason: LpObservationReason
    message: str
@dataclass(frozen=True)
class LpObservationMetadata:
    dual_feasibility: float
    stationarity: float
    complementarity: float
    objective_gap: float
    primal_check_tolerance: float
    backend_primal_tolerance: object
    backend_dual_tolerance: object
    backend: str
    backend_version: str
@dataclass(frozen=True)
class LpKktReport:
    primal_valid: bool
    dual_signs_valid: bool
    stationarity_valid: bool
    complementarity_valid: bool
    gap_valid: bool
    accepted: bool
    max_dual_sign_violation: object
    max_stationarity: object
    max_complementarity: object
    dual_objective_estimate: object
    normalized_gap: object
    message: str
@dataclass(frozen=True)
class LpRowObservation:
    active: bool
    dual_source: LpDualSource
    basis: object
    activity: object
    lower_slack: object
    upper_slack: object
    dual: object
@dataclass(frozen=True)
class LpColumnObservation:
    active: bool
    basis: object
    reduced_cost: object
@dataclass(frozen=True)
class LpObservations:
    """Copied immutable original-slot data; no solver handle or borrowed child."""
    model_id: int
    revision: int
    metadata: LpObservationMetadata
    primal_rows: LpObservationGroup
    dual_point: LpObservationGroup
    basis: LpObservationGroup
    checks: LpKktReport
    rows: tuple
    columns: tuple
    _library: Library=field(repr=False,compare=False)
    def row(self, row):
        entity=_entity(row,self._library,Row)
        if entity.model_id!=self.model_id or entity.slot>=len(self.rows) or not self.rows[entity.slot].active:
            raise ValueError("observation row is foreign, absent or deleted")
        return self.rows[entity.slot]
    def column(self, variable):
        entity=_entity(variable,self._library,Variable)
        if entity.model_id!=self.model_id or entity.slot>=len(self.columns) or not self.columns[entity.slot].active:
            raise ValueError("observation variable is foreign, absent or deleted")
        return self.columns[entity.slot]


def _lp_metadata(record):
    return {name:value for name,value in _metadata(record).items() if name not in ("struct_size","reserved_flags")}
def _lp_entry(record):
    data=_lp_metadata(record);data["active"]=bool(data["active"])
    data["basis"]=LpBasisStatus(data["basis"]) if data.pop("has_basis") else None
    if isinstance(record,_LpRow):
        data["dual_source"]=LpDualSource(data["dual_source"])
        return LpRowObservation(**data)
    return LpColumnObservation(**data)


class LpObservedResult(_Owner):
    def analyze_sensitivity(self, options=None):
        """Analyze this historical optimal basis; no optimization solve is run."""
        start=time.monotonic()
        options=LpSensitivityOptions() if options is None else options
        if not isinstance(options,LpSensitivityOptions):
            raise TypeError("options must be LpSensitivityOptions")
        native=options._marshal(self._library)
        seconds=native.time_limit_seconds
        valid_options=(native.request_count>0 and not math.isnan(seconds) and seconds>=0 and
            all(math.isfinite(getattr(native.checks,n)) and getattr(native.checks,n)>=0
                for n,_ in _SensitivityChecksOptions._fields_[2:]))
        private_token=None
        result=None
        try:
            # A private registry owner survives explicit close of the caller's
            # token after admission, while cancellation propagates both ways.
            if options.cancellation is not None:
                private_token=options.cancellation.copy()
                native.cancellation=private_token._open()
            preparation=time.monotonic()-start
            if math.isfinite(seconds) and seconds>=0:
                native.time_limit_seconds=max(0.0,seconds-preparation)
            result=_own_call(self,LpSensitivityResult,"analyze_lp_sensitivity",C.byref(native))
            native=None  # release request buffers before the publication check
            cancelled=private_token.cancelled if private_token is not None else False
            if private_token is not None:
                private_token.close();private_token=None
            result._python_stop=Termination.CANCELLED if valid_options and cancelled else None
            elapsed=time.monotonic()-start
            if valid_options and result._python_stop is None and elapsed>=seconds:
                result._python_stop=Termination.TIME_LIMIT
            result._total_elapsed_seconds=elapsed
            return result
        except BaseException:
            if result is not None:result.close()
            raise
        finally:
            if private_token is not None:private_token.close()
    _kind="lp_observed_result"
    def __init__(self, library, handle):
        self._initialize(library,handle)
    @property
    def info(self):
        out=_record(self,"lp_observed_result_info",_LpInfo);data=_lp_metadata(out)
        info=_metadata(out.result);info["termination"]=Termination(info["termination"]);info["guarantee"]=Guarantee(info["guarantee"])
        for name in ("has_solution","solution_validated","start_submitted"):
            info[name]=bool(info[name])
        data["result"]=info;data["has_observations"]=bool(data["has_observations"])
        return data
    termination=property(lambda self:self.info["result"]["termination"])
    has_solution=property(lambda self:self.info["result"]["has_solution"])
    def copy_result(self):
        return _own_call(self,Result,"lp_observed_result_copy_result")
    @property
    def observations(self):
        info=self.info
        if not info["has_observations"]:
            return None
        text=lambda field:_copied_text(self,"lp_observed_result_text",field)
        metadata=LpObservationMetadata(**_lp_metadata(_record(self,"lp_observed_result_metadata",_LpMetadata)),backend=text(0),backend_version=text(1))
        def group(index):
            data=_lp_metadata(_record(self,"lp_observed_result_group",_LpGroup,index))
            return LpObservationGroup(LpObservationState(data["state"]),LpObservationReason(data["reason"]),text(index+2))
        check=_lp_metadata(_record(self,"lp_observed_result_checks",_LpChecks))
        for name in ("primal_valid","dual_signs_valid","stationarity_valid","complementarity_valid","gap_valid","accepted"):
            check[name]=bool(check[name])
        checks=LpKktReport(**check,message=text(5))
        def entries(function,element):
            count=U64();self._library.call(function,self._open(),None,C.sizeof(element),0,C.byref(count))
            data=(element*count.value)()
            self._library.call(function,self._open(),data,C.sizeof(element),count.value,C.byref(count))
            return tuple(_lp_entry(item) for item in data)
        return LpObservations(info["model_id"],info["revision"],metadata,group(0),group(1),group(2),checks,
            entries("lp_observed_result_rows",_LpRow),entries("lp_observed_result_columns",_LpColumn),self._library)


class LpBasisOrigin(IntEnum):
    CALLER=0; OBSERVATIONS=1
class LpBasisSubmissionState(IntEnum):
    NOT_ATTEMPTED=0; ACCEPTED=1; REPAIRED=2; REJECTED=3; INTERRUPTED=4
@dataclass(frozen=True)
class LpBasisInfo:
    model_id: int
    revision: int
    row_slots: int
    column_slots: int
    origin: LpBasisOrigin
@dataclass(frozen=True)
class LpBasisSubmission:
    state: LpBasisSubmissionState
    backend_attempted: bool
    statuses_changed: object
    message: str


def _basis_solve_options(owner,basis,options):
    if not isinstance(basis,LpBasis) or basis._library is not owner._library:
        raise TypeError("basis must belong to the same ABI instance")
    if options is not None and not isinstance(options,LpObservationOptions):
        raise TypeError("options must be LpObservationOptions")
    return (options or LpObservationOptions())._marshal(owner._library)


class LpBasis(_Owner):
    """Immutable source-tagged statuses. Use a factory; close releases only this token."""
    _kind="basis"
    def __init__(self,library,handle):
        self._initialize(library,handle)
    @classmethod
    def from_model(cls,model,columns,rows):
        if not isinstance(model,Model):
            raise TypeError("model must be Model")
        def statuses(values):
            encoded=[-1 if value is None else int(LpBasisStatus(_integer(value,32,True))) for value in values]
            return (I32*len(encoded))(*encoded)
        columns=statuses(columns);rows=statuses(rows)
        return _own_call(model,cls,"basis_from_model",columns,len(columns),rows,len(rows))
    @classmethod
    def from_observed(cls,observed):
        if not isinstance(observed,LpObservedResult):
            raise TypeError("observed must be an owning LpObservedResult")
        return _own_call(observed,cls,"basis_from_observed")
    @property
    def info(self):
        data=_lp_metadata(_record(self,"basis_info",_BasisInfo))
        data["origin"]=LpBasisOrigin(data["origin"])
        return LpBasisInfo(**data)
    def _statuses(self,kind):
        n=U64();self._library.call("basis_statuses",self._open(),kind,None,0,C.byref(n))
        values=(I32*n.value)();self._library.call("basis_statuses",self._open(),kind,values,n.value,C.byref(n))
        return tuple(None if value==-1 else LpBasisStatus(value) for value in values)
    rows=property(lambda self:self._statuses(2))
    columns=property(lambda self:self._statuses(1))
    def row(self,row):
        value=I32();self._library.call("basis_row",self._open(),_entity(row,self._library,Row),C.byref(value))
        return LpBasisStatus(value.value)
    def column(self,variable):
        value=I32();self._library.call("basis_column",self._open(),_entity(variable,self._library,Variable),C.byref(value))
        return LpBasisStatus(value.value)


class LpBasisSolveResult(_Owner):
    """Submission facts and solve termination are independent; copied children own data."""
    _kind="basis_result"
    def __init__(self,library,handle):
        self._initialize(library,handle)
    @property
    def info(self):
        out=_record(self,"basis_result_info",_BasisResultInfo)
        result=_metadata(out.result);result["termination"]=Termination(result["termination"])
        result["guarantee"]=Guarantee(result["guarantee"])
        for name in ("has_solution","solution_validated","start_submitted"):
            result[name]=bool(result[name])
        return {"result":result,"requested_model_id":out.requested_model_id if out.has_requested_basis else None,
                "requested_revision":out.requested_revision if out.has_requested_basis else None}
    termination=property(lambda self:self.info["result"]["termination"])
    has_solution=property(lambda self:self.info["result"]["has_solution"])
    @property
    def submission(self):
        out=_record(self,"basis_result_info",_BasisResultInfo)
        return LpBasisSubmission(LpBasisSubmissionState(out.state),bool(out.backend_attempted),
            bool(out.statuses_changed) if out.has_statuses_changed else None,
            _copied_text(self,"basis_result_message"))
    def copy_observed(self):
        return _own_call(self,LpObservedResult,"basis_result_copy_observed")
    def copy_basis(self):
        return _own_call(self,LpBasis,"basis_result_copy_basis")


@dataclass(frozen=True)
class RegularTransition:
    """One sparse edge; state indices are zero based and symbols are signed integers."""
    from_state: int
    symbol: int
    to_state: int


class ScenarioReuse(IntEnum):
    AUTOMATIC=0; COLD=1
class ScenarioRunState(IntEnum):
    NOT_STARTED=0; ATTEMPTED=1
class ScenarioBatchCompletion(IntEnum):
    REJECTED=0; INTERRUPTED=1; COMPLETE=2
@dataclass(frozen=True)
class ScenarioId:
    batch_id: int
    index: int
    _library: Library=field(repr=False)
    def _id(self):
        return _ScenarioId(_integer(self.batch_id,64),_integer(self.index,64))
@dataclass(frozen=True)
class ScenarioVariableBounds:
    variable: Variable
    lower: object=None
    upper: object=None
@dataclass(frozen=True)
class ScenarioRowBounds:
    row: Row
    lower: object=None
    upper: object=None
@dataclass(frozen=True)
class ScenarioDefinition:
    """Absolute patches. None inherits; a zero objective coefficient removes it."""
    name: str=""
    objective_coefficients: tuple=()
    objective_offset: object=None
    variable_bounds: tuple=()
    row_bounds: tuple=()
    def __post_init__(self):
        terms=self.objective_coefficients
        if hasattr(terms,"items"):
            terms=terms.items()
        object.__setattr__(self,"objective_coefficients",tuple((v,c) for v,c in terms))
        object.__setattr__(self,"variable_bounds",tuple(self.variable_bounds))
        object.__setattr__(self,"row_bounds",tuple(self.row_bounds))
@dataclass
class ScenarioOptions:
    solve: Options=field(default_factory=Options)
    reuse: ScenarioReuse=ScenarioReuse.AUTOMATIC
    max_scenarios: int=1000
    max_patch_entries: int=1000000
    max_saved_value_slots: int=10000000
    max_work: int=100000000
    def _marshal(self,library):
        if not isinstance(self.solve,Options):
            raise TypeError("solve must be Options")
        out=_ScenarioOptions();library.call("scenario_options_default",C.byref(out),C.sizeof(out))
        solve=self.solve._marshal(library);out.solve=solve;out._keepalive=solve
        out.reuse=int(ScenarioReuse(_integer(self.reuse,32,True)))
        for name in ("max_scenarios","max_patch_entries","max_saved_value_slots","max_work"):
            setattr(out,name,_integer(getattr(self,name),64))
        return out
@dataclass(frozen=True)
class ScenarioStatistics:
    solve_calls: int
    model_loads: int
    incremental_updates: int
    unchanged_models: int
    basis_warm_starts: int
    incumbent_starts: int
@dataclass(frozen=True)
class ScenarioBatchInfo:
    model_id: int
    revision: int
    batch_id: object
    scenario_count: int
    outcome_count: int
    completion: ScenarioBatchCompletion
    stop_reason: object
    offending_scenario: object
    all_resolved: bool
    attempted: int
    resolved: int
    work: int
    elapsed_seconds: float
    reuse_statistics: ScenarioStatistics
@dataclass(frozen=True)
class ScenarioResultInfo:
    model_id: int
    revision: int
    variable_slots: int
    termination: Termination
    guarantee: Guarantee
    has_solution: bool
    solution_validated: bool
    start_submitted: bool
    elapsed_seconds: float
@dataclass(frozen=True)
class ScenarioCheck:
    identity_valid: bool
    candidate_examined: bool
    objective_matches: bool
    exact_witness_validated: bool
    validation: object
@dataclass(frozen=True)
class ScenarioOutcome:
    scenario: ScenarioId
    state: ScenarioRunState
    result: object
    check: object
    reuse_delta: ScenarioStatistics
    elapsed_seconds: float


def _scenario_definitions(definitions,library):
    data=[];keep=[]
    def bounds(values,expected,entity_type,attribute):
        result=[]
        for item in values:
            if not isinstance(item,expected):
                raise TypeError("wrong scenario bound record type")
            result.append(_ScenarioBounds(C.sizeof(_ScenarioBounds),0,_entity(getattr(item,attribute),library,entity_type),
                int(item.lower is not None),int(item.upper is not None),
                0.0 if item.lower is None else float(item.lower),0.0 if item.upper is None else float(item.upper)))
        return (_ScenarioBounds*len(result))(*result)
    for definition in definitions:
        if not isinstance(definition,ScenarioDefinition):
            raise TypeError("definitions must contain ScenarioDefinition records")
        name=_text(definition.name);terms=_terms(definition.objective_coefficients,library)
        variables=bounds(definition.variable_bounds,ScenarioVariableBounds,Variable,"variable")
        rows=bounds(definition.row_bounds,ScenarioRowBounds,Row,"row")
        offset=definition.objective_offset
        data.append(_ScenarioDefinition(C.sizeof(_ScenarioDefinition),0,name,terms,len(terms),
            _OptionalNumber(offset is not None,0,0 if offset is None else float(offset)),variables,len(variables),rows,len(rows)))
        keep.extend((name,terms,variables,rows))
    array=(_ScenarioDefinition*len(data))(*data);array._keepalive=keep;return array


class ScenarioBatchResult(_Owner):
    """Owning history. copy_result() is independent and uses private mapped IDs."""
    _kind="scenario_batch"
    def __init__(self,library,handle):
        self._initialize(library,handle)
    def _id(self,scenario):
        if not isinstance(scenario,ScenarioId):
            raise TypeError("scenario must be ScenarioId")
        if scenario._library is not self._library:
            raise ValueError("scenario belongs to another ABI instance")
        return scenario._id()
    @property
    def info(self):
        out=_record(self,"scenario_batch_info",_ScenarioInfo)
        return ScenarioBatchInfo(out.model_id,out.revision,out.batch_id if out.has_batch else None,
            out.scenario_count,out.outcome_count,ScenarioBatchCompletion(out.completion),
            Termination(out.stop_reason) if out.has_stop_reason else None,
            out.offending_scenario if out.has_offending_scenario else None,bool(out.all_resolved),out.attempted,out.resolved,
            out.work,out.elapsed_seconds,ScenarioStatistics(**_metadata(out.reuse_statistics)))
    message=property(lambda self:_copied_text(self,"scenario_batch_message"))
    def scenario(self,index):
        out=_ScenarioId();self._library.call("scenario_batch_id",self._open(),_integer(index,64),C.byref(out))
        return ScenarioId(out.batch_id,out.index,self._library)
    def check(self,scenario):
        id=self._id(scenario);out=_record(self,"scenario_batch_check",_ScenarioCheck,id)
        if not out.has_check:
            return None
        validation=None
        if out.candidate_examined:
            data=_metadata(out.validation);data["valid"]=bool(data["valid"]);data["model_valid"]=bool(data["model_valid"])
            validation=Validation(**data,message=_copied_text(self,"scenario_batch_text",id,1))
        return ScenarioCheck(bool(out.identity_valid),bool(out.candidate_examined),bool(out.objective_matches),
            bool(out.exact_witness_validated),validation)
    def outcome(self,scenario):
        out=_record(self,"scenario_batch_outcome",_ScenarioOutcome,self._id(scenario));info=None
        if out.has_result:
            data=_metadata(out.result);data["termination"]=Termination(data["termination"]);data["guarantee"]=Guarantee(data["guarantee"])
            for name in ("has_solution","solution_validated","start_submitted"):
                data[name]=bool(data[name])
            info=ScenarioResultInfo(**data)
        return ScenarioOutcome(ScenarioId(out.scenario.batch_id,out.scenario.index,self._library),ScenarioRunState(out.state),info,
            self.check(scenario),ScenarioStatistics(**_metadata(out.reuse_delta)),out.elapsed_seconds)
    def copy_result(self,scenario):
        return _own_call(self,Result,"scenario_batch_copy_result",self._id(scenario))
    def map(self,original):
        out=_Id();self._library.call("scenario_batch_map",self._open(),_entity(original,self._library,(Variable,Row)),C.byref(out))
        cls=Variable if out.kind==1 else Row
        return cls(out.model_id,out.slot,self._library)
    def value(self,scenario,original):
        out=F64();self._library.call("scenario_batch_value",self._open(),self._id(scenario),_entity(original,self._library,Variable),C.byref(out))
        return out.value
    def definition(self,scenario):
        id=self._id(scenario);out=_record(self,"scenario_batch_definition",_ScenarioDefinitionInfo,id)
        terms=_array(self,"scenario_batch_objective",_Term,id)
        def bounds(kind,cls,entity_cls):
            count=U64();self._library.call("scenario_batch_bounds",self._open(),id,kind,None,C.sizeof(_ScenarioBounds),0,C.byref(count))
            values=(_ScenarioBounds*count.value)()
            self._library.call("scenario_batch_bounds",self._open(),id,kind,values,C.sizeof(_ScenarioBounds),count.value,C.byref(count))
            return tuple(cls(entity_cls(v.entity.model_id,v.entity.slot,self._library),v.lower if v.has_lower else None,
                v.upper if v.has_upper else None) for v in values)
        return ScenarioDefinition(_copied_text(self,"scenario_batch_text",id,0),
            tuple((Variable(t.variable.model_id,t.variable.slot,self._library),t.coefficient) for t in terms),
            out.objective_offset.value if out.objective_offset.present else None,
            bounds(1,ScenarioVariableBounds,Variable),bounds(2,ScenarioRowBounds,Row))


class LpEvidenceRequest(IntEnum):
    AUTOMATIC=0; PRIMAL_RAY=1; FARKAS=2; BOTH=3
class LpEvidenceState(IntEnum):
    NOT_REQUESTED=0; AVAILABLE=1; UNAVAILABLE=2; REJECTED=3
class LpEvidenceReason(IntEnum):
    NONE=0; NOT_REQUESTED=1; UNSUPPORTED=2; NO_FEASIBLE_BASE=3; NO_IMPROVEMENT=4; NO_CONTRADICTION=5
    STOPPED=6; INVALID_BACKEND=7; FAILED_CHECKS=8; INCONSISTENT=9; INVALID_MODEL=10; RESOURCE_LIMIT=11; ALLOCATION=12
class LpEvidenceCompletion(IntEnum):
    COMPLETE=0; INTERRUPTED=1; REJECTED=2
class LpEvidencePhase(IntEnum):
    FEASIBLE_BASE=0; RECESSION=1; FARKAS=2
class LpEvidenceSide(IntEnum):
    LOWER=0; UPPER=1
class LpEvidenceColumnKind(IntEnum):
    SOURCE_VARIABLE=0; ROW_SIDE=1; VARIABLE_SIDE=2
@dataclass
class LpEvidenceOptions:
    solve: Options=field(default_factory=Options)
    request: LpEvidenceRequest=LpEvidenceRequest.AUTOMATIC
    recession: float=1e-7
    stationarity: float=1e-7
    minimum_improvement: float=1e-7
    minimum_contradiction: float=1e-7
    max_auxiliary_variables: int=1000000
    max_auxiliary_rows: int=1000000
    max_auxiliary_nonzeros: int=10000000
    max_retained_slots: int=50000000
    max_work: int=100000000
    max_auxiliary_solves: int=3
    def _marshal(self,library):
        if not isinstance(self.solve,Options):
            raise TypeError("solve must be Options")
        out=_EvidenceOptions();library.call("evidence_options_default",C.byref(out),C.sizeof(out))
        solve=self.solve._marshal(library);out.solve=solve;out._keepalive=solve
        out.request=int(LpEvidenceRequest(_integer(self.request,32,True)))
        for name in ("recession","stationarity","minimum_improvement","minimum_contradiction"):
            setattr(out,name,float(getattr(self,name)))
        for name in ("max_auxiliary_variables","max_auxiliary_rows","max_auxiliary_nonzeros","max_retained_slots","max_work","max_auxiliary_solves"):
            setattr(out,name,_integer(getattr(self,name),64))
        return out
@dataclass(frozen=True)
class LpEvidenceInfo:
    model_id: int
    revision: int
    has_evidence: bool
    completion: LpEvidenceCompletion
    stop_reason: object
    row_slots: int
    column_slots: int
    stage_count: int
    attempted_calls: int
    work: int
    elapsed_seconds: float
@dataclass(frozen=True)
class LpEvidenceGroup:
    state: LpEvidenceState
    reason: LpEvidenceReason
    message: str
@dataclass(frozen=True)
class LpEvidenceMetadata:
    recession: float
    stationarity: float
    minimum_improvement: float
    minimum_contradiction: float
    primal_tolerance: float
@dataclass(frozen=True)
class LpPrimalEvidence:
    base_check: object
    direction_scale: object
    normalized_objective_slope: object
    max_variable_recession_violation: object
    max_row_recession_violation: object
@dataclass(frozen=True)
class LpFarkasEvidence:
    multiplier_scale: object
    contradiction_margin: object
    max_stationarity: object
@dataclass(frozen=True)
class LpEvidenceSlot:
    """Original-slot diagnostics; consult group state for accepted evidence."""
    source: object
    active: bool
    side: object
    base_value: object
    direction: object
    multiplier: object
    contribution: object
    selected_bound: object
@dataclass(frozen=True)
class LpEvidenceDiagnostics:
    metadata: LpEvidenceMetadata
    primal: LpPrimalEvidence
    farkas: LpFarkasEvidence
    columns: tuple
    rows: tuple
@dataclass(frozen=True)
class LpEvidenceRawResult:
    """Untrusted reported auxiliary fields; deliberately no has_solution API."""
    model_id: int
    revision: int
    value_count: int
    mask_count: int
    termination_code: int
    guarantee_code: int
    reported_solution_validated: bool
    reported_start_submitted: bool
    elapsed_seconds: float
    objective: object
    best_bound: object
    absolute_gap: object
    relative_gap: object
    native_gap: object
@dataclass(frozen=True)
class LpEvidenceStageInfo:
    index: int
    private_model_id: int
    private_revision: int
    row_count: int
    column_count: int
    nonzeros: int
    phase: LpEvidencePhase
    attempted: bool
    candidate_examined: bool
    check: object
    raw_result: object
@dataclass(frozen=True)
class LpEvidenceColumn:
    private_variable: Variable
    source: object
    kind: LpEvidenceColumnKind
    side: object
@dataclass(frozen=True)
class LpEvidenceRawValue:
    slot: int
    reported_value: object
    reported_mask: object


def _evidence_validation(record,message):
    data=_metadata(record);data["valid"]=bool(data["valid"]);data["model_valid"]=bool(data["model_valid"])
    return Validation(**data,message=message)

def _evidence_source(record,library):
    if record.reserved or record.kind not in (1,2):
        raise RuntimeError("invalid evidence source ID")
    return (Variable if record.kind==1 else Row)(record.model_id,record.slot,library)

def _evidence_slot_record(record,library):
    data=_lp_metadata(record);data["source"]=_evidence_source(record.source,library);data["active"]=bool(data["active"])
    data["side"]=LpEvidenceSide(data["side"]) if data.pop("has_side") else None
    return LpEvidenceSlot(**data)

def _evidence_array(owner,function,element,*args):
    count=U64();owner._library.call(function,owner._open(),*args,None,C.sizeof(element),0,C.byref(count))
    out=(element*count.value)();owner._library.call(function,owner._open(),*args,out,C.sizeof(element),count.value,C.byref(count))
    return out


class LpEvidenceResult(_Owner):
    """Owning numerical evidence analysis; not an original optimization Result."""
    _kind="lp_evidence"
    def __init__(self,library,handle):
        self._initialize(library,handle)
    @property
    def info(self):
        out=_record(self,"lp_evidence_info",_EvidenceInfo);data=_lp_metadata(out)
        data["has_evidence"]=bool(data["has_evidence"]);data["completion"]=LpEvidenceCompletion(data["completion"])
        data["stop_reason"]=Termination(data["stop_reason"]) if data.pop("has_stop_reason") else None
        return LpEvidenceInfo(**data)
    message=property(lambda self:_copied_text(self,"lp_evidence_text",0))
    def _group(self,index):
        if not self.info.has_evidence:
            return None
        out=_record(self,"lp_evidence_group",_EvidenceGroup,index)
        return LpEvidenceGroup(LpEvidenceState(out.state),LpEvidenceReason(out.reason),_copied_text(self,"lp_evidence_text",index+1))
    primal_ray=property(lambda self:self._group(0))
    farkas=property(lambda self:self._group(1))
    @property
    def diagnostics(self):
        if not self.info.has_evidence:
            return None
        metadata=LpEvidenceMetadata(**_lp_metadata(_record(self,"lp_evidence_metadata",_EvidenceMetadata)))
        p=_record(self,"lp_evidence_primal",_EvidencePrimal);data=_lp_metadata(p)
        data["base_check"]=_evidence_validation(p.base_check,_copied_text(self,"lp_evidence_text",3)) if data.pop("has_base_check") else None
        primal=LpPrimalEvidence(**data);farkas=LpFarkasEvidence(**_lp_metadata(_record(self,"lp_evidence_farkas",_EvidenceFarkas)))
        slots=lambda kind:tuple(_evidence_slot_record(r,self._library) for r in _evidence_array(self,"lp_evidence_slots",_EvidenceSlot,kind))
        return LpEvidenceDiagnostics(metadata,primal,farkas,slots(1),slots(2))
    def slot(self,source):
        out=_record(self,"lp_evidence_slot",_EvidenceSlot,_entity(source,self._library,(Variable,Row)))
        return _evidence_slot_record(out,self._library)
    def _value(self,variable,field):
        out=F64();self._library.call("lp_evidence_value",self._open(),_entity(variable,self._library,Variable),field,C.byref(out));return out.value
    def base_value(self,variable):
        """Requires Available primal-ray group; diagnostics retain base-only data."""
        return self._value(variable,0)
    def direction_value(self,variable):
        return self._value(variable,1)
    def multiplier(self,source):
        """Requires Available Farkas group; slot() provides raw diagnostics."""
        return _evidence_slot_record(_record(self,"lp_evidence_multiplier",_EvidenceSlot,_entity(source,self._library,(Variable,Row))),self._library)
    def copy_stage(self,index):
        return _own_call(self,LpEvidenceStage,"lp_evidence_copy_stage",_integer(index,64))


class LpEvidenceStage(_Owner):
    """Independent owning child: raw auxiliary diagnostics, never a Result."""
    _kind="lp_evidence_stage"
    def __init__(self,library,handle):
        self._initialize(library,handle)
    @property
    def info(self):
        out=_record(self,"lp_evidence_stage_info",_EvidenceStage);data=_lp_metadata(out)
        data["phase"]=LpEvidencePhase(data["phase"]);data["attempted"]=bool(data["attempted"])
        data["candidate_examined"]=bool(data["candidate_examined"])
        data["check"]=_evidence_validation(out.check,_copied_text(self,"lp_evidence_stage_text",3)) if out.candidate_examined else None
        if data.pop("has_raw_result"):
            raw=_lp_metadata(out.raw_result)
            raw["reported_solution_validated"]=bool(raw["reported_solution_validated"])
            raw["reported_start_submitted"]=bool(raw["reported_start_submitted"])
            data["raw_result"]=LpEvidenceRawResult(**raw)
        else:data["raw_result"]=None
        return LpEvidenceStageInfo(**data)
    @property
    def columns(self):
        out=[]
        for r in _evidence_array(self,"lp_evidence_stage_columns",_EvidenceColumn):
            out.append(LpEvidenceColumn(_evidence_source(r.private_variable,self._library),_evidence_source(r.source,self._library),
                LpEvidenceColumnKind(r.kind),LpEvidenceSide(r.side) if r.has_side else None))
        return tuple(out)
    @property
    def raw_values(self):
        return tuple(LpEvidenceRawValue(r.slot,r.reported_value.value if r.reported_value.present else None,
            r.reported_mask if r.has_reported_mask else None) for r in _evidence_array(self,"lp_evidence_stage_raw_values",_EvidenceRawValue))
    raw_backend=property(lambda self:_copied_text(self,"lp_evidence_stage_text",0))
    raw_backend_version=property(lambda self:_copied_text(self,"lp_evidence_stage_text",1))
    raw_message=property(lambda self:_copied_text(self,"lp_evidence_stage_text",2))


class LpSensitivityState(IntEnum):
    NOT_REQUESTED=0; AVAILABLE=1; UNAVAILABLE=2; REJECTED=3
class LpSensitivityReason(IntEnum):
    NONE=0; NOT_REQUESTED=1; UNSUPPORTED=2; NOT_OPTIMAL=3; NO_BASIS=4; INVALID_SOURCE=5
    INVALID_BASIS=6; CHANGED_BASIS=7; REFERENCE_CHECKS=8; SYSTEM_CHECKS=9; INTERVAL_CHECKS=10
    RESOURCE_LIMIT=11; STOPPED=12; ALLOCATION=13; BACKEND=14
class LpSensitivityCompletion(IntEnum):
    COMPLETE=0; PARTIAL=1; INTERRUPTED=2; REJECTED=3
class LpRangeEndKind(IntEnum):
    FINITE=0; NEGATIVE_INFINITY=1; POSITIVE_INFINITY=2
class LpSensitivitySide(IntEnum):
    LOWER=0; UPPER=1; FIXED=2; FREE=3
@dataclass(frozen=True)
class LpObjectiveParameter:
    variable: Variable
@dataclass(frozen=True)
class LpEqualityRhsParameter:
    row: Row
@dataclass(frozen=True)
class LpSensitivityTolerances:
    primal_feasibility: float=1e-7
    dual_feasibility: float=1e-7
    stationarity: float=1e-7
    complementarity: float=1e-6
    objective_gap: float=1e-6
    system_absolute: float=1e-9
    system_relative: float=1e-9
@dataclass(frozen=True)
class LpSensitivityLimits:
    max_rows: int=4096
    max_columns: int=100000
    max_nonzeros: int=1000000
    max_requests: int=4096
    max_basis_solves: int=8194
    max_factor_entries: int=16777216
    max_retained_slots: int=20000000
    max_work: int=100000000
@dataclass(frozen=True)
class LpSensitivityOptions:
    parameters: tuple=()
    backend: Backend=Backend.AUTO
    time_limit_seconds: float=math.inf
    cancellation: object=None
    checks: LpSensitivityTolerances=field(default_factory=LpSensitivityTolerances)
    limits: LpSensitivityLimits=field(default_factory=LpSensitivityLimits)
    def _marshal(self,library):
        if not isinstance(self.parameters,(tuple,list)):
            raise TypeError("parameters must be a tuple or list")
        if not isinstance(self.checks,LpSensitivityTolerances) or not isinstance(self.limits,LpSensitivityLimits):
            raise TypeError("sensitivity checks/limits have the wrong type")
        out=_SensitivityOptions();library.call("sensitivity_options_default",C.byref(out),C.sizeof(out))
        out.backend=int(Backend(_integer(self.backend,32,True)));out.time_limit_seconds=float(self.time_limit_seconds)
        if self.cancellation is not None:
            if not isinstance(self.cancellation,Cancellation) or self.cancellation._library is not library:
                raise TypeError("cancellation belongs to another library or has the wrong type")
            out.cancellation=self.cancellation._open()
        for name,_ in _SensitivityChecksOptions._fields_[2:]:
            setattr(out.checks,name,float(getattr(self.checks,name)))
        for name,_ in _SensitivityLimits._fields_[2:]:
            setattr(out.limits,name,_integer(getattr(self.limits,name),64))
        n=_integer(len(self.parameters),64);out.request_count=n
        if n>out.limits.max_requests or n>out.limits.max_work:
            # The v1 C admission gate checks these counts before dereferencing
            # any element. A bounded sentinel allows an owning quota rejection
            # without allocating a buffer already known to exceed the quota.
            entries=(_SensitivityRequest*1)()
        else:
            entries=(_SensitivityRequest*n)()
            for i,p in enumerate(self.parameters):
                entries[i].struct_size=C.sizeof(_SensitivityRequest)
                if isinstance(p,LpObjectiveParameter):
                    entries[i].kind=0;entries[i].entity=_entity(p.variable,library,Variable)
                elif isinstance(p,LpEqualityRhsParameter):
                    entries[i].kind=1;entries[i].entity=_entity(p.row,library,Row)
                else:raise TypeError("unknown sensitivity parameter descriptor")
        out.requests=entries;out._keepalive=(entries,self.cancellation)
        return out
@dataclass(frozen=True)
class LpSensitivityInfo:
    model_id: int
    revision: int
    completion: LpSensitivityCompletion
    reason: LpSensitivityReason
    stop_reason: object
    has_sensitivity: bool
    has_basis: bool
    guarantee: Guarantee
    entry_count: int
    factor_order_count: int
    row_slots: int
    column_slots: int
    elapsed_seconds: float
@dataclass(frozen=True)
class LpSensitivityWork:
    factor_setup_attempted: bool
    basis_solves: int
    coordinator_visits: int
    retained_slots: int
    preparation_visits: int
@dataclass(frozen=True)
class LpSensitivityGroup:
    state: LpSensitivityState
    reason: LpSensitivityReason
    message: str
@dataclass(frozen=True)
class LpRangeEnd:
    kind: LpRangeEndKind
    value: object
    def __post_init__(self):
        if not isinstance(self.kind,LpRangeEndKind):raise TypeError("endpoint kind must be LpRangeEndKind")
        if self.kind is LpRangeEndKind.FINITE:
            if self.value is None or not math.isfinite(self.value):raise ValueError("finite endpoint requires a finite value")
        elif self.value is not None:raise ValueError("infinite endpoint has no numeric value")
@dataclass(frozen=True)
class LpSensitivityLimiter:
    entity: object
    side: LpSensitivitySide
    dual_condition: bool
@dataclass(frozen=True)
class LpIntervalCheckReport:
    accepted: bool
    inequalities: int
    max_endpoint_violation: object
    lower_direction_checked: bool
    upper_direction_checked: bool
    message: str
@dataclass(frozen=True)
class LpParameterInterval:
    anchor: float
    lower: LpRangeEnd
    upper: LpRangeEnd
    objective_slope: object
    lower_limiter: object
    upper_limiter: object
    checks: LpIntervalCheckReport
@dataclass(frozen=True)
class LpSensitivityEntry:
    index: int
    parameter: object
    group: LpSensitivityGroup
    interval: object
@dataclass(frozen=True)
class LpSensitivityReferenceChecks:
    """Copied diagnostics; default false flags do not prove a check ran."""
    primal: Validation
    kkt: LpKktReport
    basis_point_matches: bool
    max_point_difference: object
    max_system_residual: object
    max_scaled_system_residual: object


class LpSensitivityResult(_Owner):
    """Owning fixed-basis numerical ranges, distinct from the source solve."""
    _kind="sensitivity"
    def __init__(self,library,handle):
        self._initialize(library,handle)
    def _text(self,field,index=0):
        return _copied_text(self,"sensitivity_text",field,index)
    @property
    def info(self):
        out=_record(self,"sensitivity_info",_SensitivityInfo);data=_lp_metadata(out)
        data["completion"]=LpSensitivityCompletion(data["completion"]);data["reason"]=LpSensitivityReason(data["reason"])
        data["stop_reason"]=Termination(data["stop_reason"]) if data.pop("has_stop_reason") else None
        data["has_sensitivity"]=bool(data["has_sensitivity"]);data["has_basis"]=bool(data["has_basis"])
        data["guarantee"]=Guarantee(data["guarantee"])
        if getattr(self,"_python_stop",None) is not None:
            data.update(completion=LpSensitivityCompletion.INTERRUPTED,reason=LpSensitivityReason.STOPPED,stop_reason=self._python_stop)
        data["elapsed_seconds"]=getattr(self,"_total_elapsed_seconds",data["elapsed_seconds"])
        return LpSensitivityInfo(**data)
    @property
    def message(self):
        self._open()
        if getattr(self,"_python_stop",None) is not None:return "Whole Python sensitivity allowance stopped during input cleanup"
        return self._text(0)
    @property
    def work(self):
        data=_lp_metadata(_record(self,"sensitivity_work",_SensitivityWork));data["factor_setup_attempted"]=bool(data["factor_setup_attempted"])
        return LpSensitivityWork(**data)
    def copy_source_observed(self):
        return _own_call(self,LpObservedResult,"sensitivity_copy_source_observed")
    def copy_basis(self):
        return _own_call(self,LpBasis,"sensitivity_copy_basis")
    @property
    def checks_options(self):
        if not self.info.has_sensitivity:return None
        return LpSensitivityTolerances(**_lp_metadata(_record(self,"sensitivity_checks_options",_SensitivityChecksOptions)))
    @property
    def reference_checks(self):
        if not self.info.has_sensitivity:return None
        r=_record(self,"sensitivity_reference_checks",_SensitivityReferenceChecks)
        primal=_evidence_validation(r.primal,self._text(4));kkt=_lp_metadata(r.kkt)
        for name in ("primal_valid","dual_signs_valid","stationarity_valid","complementarity_valid","gap_valid","accepted"):
            kkt[name]=bool(kkt[name])
        return LpSensitivityReferenceChecks(primal,LpKktReport(**kkt,message=self._text(5)),bool(r.basis_point_matches),
            *[getattr(r,n).value if getattr(r,n).present else None for n in
              ("max_point_difference","max_system_residual","max_scaled_system_residual")])
    def _entry(self,r):
        if not r.requested:return None
        entity=_evidence_source(r.request.entity,self._library)
        parameter=LpObjectiveParameter(entity) if r.request.kind==0 else LpEqualityRhsParameter(entity)
        if getattr(self,"_python_stop",None) is not None:
            return LpSensitivityEntry(r.index,parameter,LpSensitivityGroup(LpSensitivityState.UNAVAILABLE,
                LpSensitivityReason.STOPPED,self.message),None)
        group=LpSensitivityGroup(LpSensitivityState(r.group.state),LpSensitivityReason(r.group.reason),self._text(2,r.index))
        interval=None
        if r.has_interval:
            end=lambda e:LpRangeEnd(LpRangeEndKind(e.kind),e.value.value if e.value.present else None)
            limiter=lambda l:LpSensitivityLimiter(_evidence_source(l.entity,self._library),LpSensitivitySide(l.side),bool(l.dual_condition))
            c=r.checks;checks=LpIntervalCheckReport(bool(c.accepted),c.inequalities,
                c.max_endpoint_violation.value if c.max_endpoint_violation.present else None,
                bool(c.lower_direction_checked),bool(c.upper_direction_checked),self._text(3,r.index))
            interval=LpParameterInterval(r.anchor,end(r.lower),end(r.upper),r.objective_slope.value if r.objective_slope.present else None,
                limiter(r.lower_limiter) if r.has_lower_limiter else None,limiter(r.upper_limiter) if r.has_upper_limiter else None,checks)
        return LpSensitivityEntry(r.index,parameter,group,interval)
    def entry(self,index):
        return self._entry(_record(self,"sensitivity_entry",_SensitivityEntry,_integer(index,64)))
    @property
    def entries(self):
        if not self.info.has_sensitivity:return None
        return tuple(self._entry(r) for r in _evidence_array(self,"sensitivity_entries",_SensitivityEntry))
    def objective(self,variable):
        return self._entry(_record(self,"sensitivity_objective",_SensitivityEntry,_entity(variable,self._library,Variable)))
    def equality_rhs(self,row):
        return self._entry(_record(self,"sensitivity_equality_rhs",_SensitivityEntry,_entity(row,self._library,Row)))
    @property
    def factor_order(self):
        if not self.info.has_sensitivity:return None
        return tuple(_evidence_source(r,self._library) for r in _array(self,"sensitivity_factor_order",_Id))
    def active_slots(self,entity_type):
        if entity_type not in (Variable,Row):raise TypeError("entity_type must be Variable or Row")
        return tuple(bool(v) for v in _array(self,"sensitivity_active_slots",U8,1 if entity_type is Variable else 2))
    @property
    def backend_version(self):
        return self._text(1) if self.info.has_sensitivity else None
