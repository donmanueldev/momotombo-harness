#!/usr/bin/env bats

load 'test_helper'

setup() {
    setup_test_env
    CONTROLLER="$PROJECT_DIR/scripts/orchestration-controller.py"
    RUN_ROOT="$FAKE_REPO"
    cat > "$FAKE_REPO/spec.json" <<'JSON'
{"agent":"specifier","approval_status":"approved","approval_evidence":"user-ref","specification_id":"SPEC-AO","specification_revision":1,"requirements":[{"id":"REQ-AO-1"},{"id":"REQ-AO-2"}]}
JSON
}

teardown() { teardown_test_env; }

run_controller() {
    run python3 "$CONTROLLER" --root "$RUN_ROOT" "$@"
}

start_run() { run_controller start --run-id run-1 --spec "$FAKE_REPO/spec.json"; [[ "$status" -eq 0 ]]; }

approve_to_execute() {
    start_run
    run_controller advance --run-id run-1 --to specify; [[ "$status" -eq 0 ]]
    run_controller advance --run-id run-1 --to awaiting_approval; [[ "$status" -eq 0 ]]
    run_controller approve --run-id run-1 --evidence user-ref; [[ "$status" -eq 0 ]]
    cat > "$FAKE_REPO/planner.json" <<'JSON'
{"agent":"planner","status":"ok","goal":"plan","decision":"ready","result":"plan recorded","evidence":[],"specification_id":"SPEC-AO","specification_revision":1,"requirement_plan":[{"requirement_id":"REQ-AO-1"},{"requirement_id":"REQ-AO-2"}]}
JSON
    run_controller record-output --run-id run-1 --agent planner --input "$FAKE_REPO/planner.json"; [[ "$status" -eq 0 ]]
    run_controller advance --run-id run-1 --to route; [[ "$status" -eq 0 ]]
    cat > "$FAKE_REPO/route.json" <<'JSON'
{"agent":"orchestrator","status":"ok","goal":"route","decision":"ready","result":"route recorded","evidence":[],"specification_id":"SPEC-AO","specification_revision":1,"requirement_assignments":[{"requirement_id":"REQ-AO-1","owner":"backend","scope":"scripts/a.py"},{"requirement_id":"REQ-AO-2","owner":"backend","scope":"scripts/b.py"}]}
JSON
    run_controller record-output --run-id run-1 --agent orchestrator --input "$FAKE_REPO/route.json"; [[ "$status" -eq 0 ]]
    run_controller advance --run-id run-1 --to execute; [[ "$status" -eq 0 ]]
}

@test "controller rejects planning before recorded human approval" {
    start_run
    run_controller advance --run-id run-1 --to plan
    [[ "$status" -ne 0 ]]
    [[ "$output" == *"illegal transition"* ]]
}

@test "controller records scoped authorization and blocks prohibited or out-of-scope dispatch" {
    approve_to_execute
    run_controller dispatch --run-id run-1 --assignment-id no-auth --owner backend --requirements REQ-AO-1 --paths scripts/a.py --authorization-id missing --effect local_write
    [[ "$status" -ne 0 ]]
    [[ "$output" == *"no valid scoped authorization"* ]]

    run_controller authorize --run-id run-1 --authorization-id approval-1 --requirements REQ-AO-1 --paths scripts/a.py --evidence user-ref --expires-at 2030-01-01T00:00:00Z
    [[ "$status" -eq 0 ]]
    run_controller dispatch --run-id run-1 --assignment-id bad-effect --owner backend --requirements REQ-AO-1 --paths scripts/a.py --authorization-id approval-1 --effect push
    [[ "$status" -ne 0 ]]
    [[ "$output" == *"prohibited"* ]]
    run_controller dispatch --run-id run-1 --assignment-id wrong-path --owner backend --requirements REQ-AO-1 --paths scripts/b.py --authorization-id approval-1 --effect local_write
    [[ "$status" -ne 0 ]]
    [[ "$output" == *"does not match the validated route assignment"* ]]
}

@test "controller rejects nested, read-only, duplicate, and overlapping dispatch" {
    approve_to_execute
    run_controller authorize --run-id run-1 --authorization-id approval-1 --requirements REQ-AO-1,REQ-AO-2 --paths scripts/a.py,scripts/a.py/child.py --evidence user-ref --expires-at 2030-01-01T00:00:00Z
    [[ "$status" -eq 0 ]]
    run_controller dispatch --run-id run-1 --assignment-id nested --owner backend --requirements REQ-AO-1 --paths scripts/a.py --authorization-id approval-1 --effect local_write --parent orchestrator
    [[ "$status" -ne 0 ]]
    run_controller dispatch --run-id run-1 --assignment-id readonly --owner reviewer --requirements REQ-AO-1 --paths scripts/a.py --authorization-id approval-1 --effect local_write
    [[ "$status" -ne 0 ]]
    run_controller dispatch --run-id run-1 --assignment-id backend-1 --owner backend --requirements REQ-AO-1 --paths scripts/a.py --authorization-id approval-1 --effect local_write
    [[ "$status" -eq 0 ]]
    run_controller dispatch --run-id run-1 --assignment-id duplicate --owner web --requirements REQ-AO-1 --paths scripts/a.py/child.py --authorization-id approval-1 --effect local_write
    [[ "$status" -ne 0 ]]
    [[ "$output" == *"already have an owner"* ]]
}

@test "controller completes only after exact verified reviewer coverage and resumes safely" {
    approve_to_execute
    run_controller authorize --run-id run-1 --authorization-id approval-1 --requirements REQ-AO-1,REQ-AO-2 --paths scripts/a.py,scripts/b.py --evidence user-ref --expires-at 2030-01-01T00:00:00Z; [[ "$status" -eq 0 ]]
    run_controller dispatch --run-id run-1 --assignment-id backend-1 --owner backend --requirements REQ-AO-1 --paths scripts/a.py --authorization-id approval-1 --effect local_write; [[ "$status" -eq 0 ]]
    run_controller dispatch --run-id run-1 --assignment-id backend-2 --owner backend --requirements REQ-AO-2 --paths scripts/b.py --authorization-id approval-1 --effect local_write; [[ "$status" -eq 0 ]]
    run_controller complete-dispatch --run-id run-1 --assignment-id backend-1; [[ "$status" -eq 0 ]]
    run_controller complete-dispatch --run-id run-1 --assignment-id backend-2; [[ "$status" -eq 0 ]]
    cat > "$FAKE_REPO/review.json" <<'JSON'
{"agent":"reviewer","status":"ok","goal":"review","decision":"ready","result":"review recorded","evidence":[],"specification_id":"SPEC-AO","specification_revision":1,"requirement_coverage":[{"requirement_id":"REQ-AO-1","status":"verified","evidence":"test"},{"requirement_id":"REQ-AO-2","status":"verified","evidence":"test"}]}
JSON
    run_controller advance --run-id run-1 --to review; [[ "$status" -eq 0 ]]
    run_controller record-output --run-id run-1 --agent reviewer --input "$FAKE_REPO/review.json"; [[ "$status" -eq 0 ]]
    run_controller advance --run-id run-1 --to verify; [[ "$status" -eq 0 ]]
    run_controller advance --run-id run-1 --to completed; [[ "$status" -eq 0 ]]
    run_controller resume --run-id run-1
    [[ "$status" -eq 0 ]]
    [[ "$output" == *'"state": "completed"'* || "$output" == *'"state":"completed"'* ]]
    run_controller advance --run-id run-1 --to verify
    [[ "$status" -ne 0 ]]
}

@test "controller validates durable monotonic journals and cancellation records a reason" {
    start_run
    run_controller validate --run-id run-1
    [[ "$status" -eq 0 ]]
    run_controller cancel --run-id run-1 --reason "human pause" --next-step "obtain direction"
    [[ "$status" -eq 0 ]]
    run python3 -c 'import json,sys; state=json.load(open(sys.argv[1])); assert state["state"] == "cancelled"; assert state["terminal_reason"] == "human pause"' "$FAKE_REPO/.codex/orchestration/runs/run-1/state.json"
    [[ "$status" -eq 0 ]]
}

@test "controller rejects phase bypasses, non-ready reviews, and terminal mutations" {
    approve_to_execute
    run_controller advance --run-id run-1 --to review
    [[ "$status" -ne 0 ]]
    [[ "$output" == *"completed dispatch"* ]]
    run_controller stop --run-id run-1 --to blocked --reason "missing evidence" --next-step "supply route evidence"
    [[ "$status" -eq 0 ]]
    run_controller authorize --run-id run-1 --authorization-id late --requirements REQ-AO-1 --paths scripts/a.py --evidence user-ref --expires-at 2030-01-01T00:00:00Z
    [[ "$status" -ne 0 ]]
    [[ "$output" == *"terminal runs cannot mutate"* ]]
}
