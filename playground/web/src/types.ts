export interface Example {
  id: string;
  name: string;
  description: string;
  source: string;
  path: string;
  arguments: string[];
}
export interface Health {
  ready: boolean;
  compiler: string;
  compiler_sha256: string;
  artifact_root: string;
  checks: Record<string, boolean>;
  version: string;
  repair_command: string;
}
export interface PassStats {
  function: string;
  pass: string;
  requires: string;
  invalidates: string;
  changed_instructions: number;
  before: number;
  after: number;
  ns: number;
}
export interface PassSnapshot {
  key: string;
  order: number;
  pass: string;
  stats?: PassStats;
}
export interface TraceFunction {
  id: number;
  name: string;
  snapshots: PassSnapshot[];
}
export interface Diagnostic {
  file: string;
  line: number;
  column: number;
  message: string;
  editable: boolean;
}
export interface ProcessResult {
  command: string[];
  returncode: number;
  seconds: number;
  stdout: string;
  stderr: string;
  timed_out: boolean;
  truncated: boolean;
}
export interface Job {
  id: string;
  source: string;
  source_sha256: string;
  optimization: number;
  llvm_optimization: number;
  status: string;
  error?: string;
  commands: ProcessResult[];
  diagnostics: Diagnostic[];
  functions: TraceFunction[];
  stats: PassStats[];
  artifacts: string[];
  execution?: ProcessResult;
}
export interface TraceDataset {
  id: string;
  name: string;
  available: boolean;
  source: string;
}
export interface Instruction {
  id: string;
  type: string;
  op: string;
  text: string;
  line: number;
  uses: { value: string; predecessor: string | null }[];
}
export interface Block {
  id: string;
  line: number;
  instructions: Instruction[];
  terminator: string;
  terminator_line: number;
  terminator_uses: string[];
  edges: { target: string; label: string }[];
}
export interface Snapshot {
  text: string;
  sha256: string;
  name: string;
  blocks: Block[];
  instruction_count: number;
}
export interface ASTNode {
  id: number;
  tag: string;
  text: string;
  type: string;
  a: number;
  b: number;
  c: number;
  children: number[];
  file: number;
  offset: number;
}
export interface AST {
  records: { name: string; fields: { name: string; type: string }[] }[];
  functions: {
    name: string;
    parameters: { name: string; type: string }[];
    return: string;
    body: number;
  }[];
  nodes: ASTNode[];
}
export interface BootstrapStage {
  optimization: number;
  stage: number;
  producer: string;
  binary: string;
  binary_sha256: string;
  llvm_sha256: string;
  binary_bytes: number;
  seconds: number;
  trace_files: number;
  current_binary_sha256: string | null;
  current_llvm_sha256: string | null;
  current_matches: boolean;
}
export interface TestResult {
  case: string;
  optimization?: number;
  status: string;
  stdout?: string;
  stderr?: string;
  seconds?: number;
}
export interface TestRecord {
  compiler: string;
  passed: number;
  results: TestResult[];
  cross_stage_artifacts_equal?: boolean;
}
export interface BenchmarkRow {
  benchmark: string;
  configuration: string;
  input: string[];
  instructions: number;
  blocks: number;
  compile_ms: number;
  runtime_ms: number;
  samples_ms: number[];
  binary_bytes: number;
  stdout: string;
  output_sha256: string;
  pass_stats: PassStats[];
}
export interface DashboardData {
  records: {
    bootstrap: {
      fixed_point_O0: boolean;
      fixed_point_O2: boolean;
      cross_stage_traces_equal: boolean;
      standalone: {
        seed_hidden: boolean;
        relocated: boolean;
        stdout: string;
        no_libstdcpp: boolean;
      };
    } | null;
    tests: TestRecord | null;
    'tests-stage3': TestRecord | null;
    benchmarks: { warmups: number; measurements: number; rows: BenchmarkRow[] } | null;
    sanitizers: { status: string; seed_checked_own_source: boolean; runtime_case: string } | null;
    optimization: { status: string; verified: string[] } | null;
    toolchain: Record<string, unknown> | null;
    'trace-hashes': Record<string, Record<string, string>> | null;
  };
  stages: BootstrapStage[];
  seed: { path: string; sha256: string | null; bytes: number | null };
  provenance: { path: string; matches: boolean }[];
  sources: { key: string; path: string; available: boolean; modified: number | null }[];
}
