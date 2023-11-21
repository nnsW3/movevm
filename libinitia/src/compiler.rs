use std::{path::Path, str::FromStr};

use crate::{error::Error, ByteSliceView};

use initia_compiler::{compile as initia_compile, prover::ProverOptions};
use log::LevelFilter;
use move_cli::{base::test::Test, Move};
use move_package::{Architecture, BuildConfig, CompilerConfig};

pub use initia_compiler::Command;

pub fn compile(move_args: Move, cmd: Command) -> Result<Vec<u8>, Error> {
    let action = cmd.to_string();
    let verbose = move_args.verbose;

    match initia_compile(move_args, cmd) {
        Ok(_) => Ok(Vec::from("ok")),
        Err(e) => {
            if verbose {
                Err(Error::backend_failure(format!(
                    "failed to {}: {:?}",
                    action, e
                )))
            } else {
                Err(Error::backend_failure(format!(
                    "failed to {}: {}",
                    action, e
                )))
            }
        }
    }
}

/// cbindgen:prefix-with-name
#[allow(dead_code)]
#[derive(PartialEq)]
#[repr(u8)] // This makes it so the enum looks like a simple u32 to Go
pub enum CoverageOption {
    /// Display a coverage summary for all modules in this package
    Summary = 0, // no 0 for the purpose
    /// Display coverage information about the module against source code
    Source = 1,
    /// Display coverage information about the module against disassembled bytecode
    Bytecode = 2,
}

#[repr(C)]
pub struct InitiaCompilerArgument {
    /// Path to a package which the command should be run with respect to.
    pub package_path: ByteSliceView,

    /// Print additional diagnostics if available.
    pub verbose: bool,

    /// Package build options
    pub build_config: InitiaCompilerBuildConfig,
}

impl From<InitiaCompilerArgument> for Move {
    fn from(val: InitiaCompilerArgument) -> Self {
        let package_path = val
            .package_path
            .read()
            .map(|s| Path::new(&String::from_utf8(s.to_vec()).unwrap()).to_path_buf());
        Self {
            package_path,
            verbose: val.verbose,
            build_config: val.build_config.into(),
        }
    }
}

#[repr(C)]
pub struct InitiaCompilerBuildConfig {
    /// Compile in 'dev' mode. The 'dev-addresses' and 'dev-dependencies' fields will be used if
    /// this flag is set. This flag is useful for development of packages that expose named
    /// addresses that are not set to a specific value.
    pub dev_mode: bool,
    /// Compile in 'test' mode. The 'dev-addresses' and 'dev-dependencies' fields will be used
    /// along with any code in the 'tests' directory.
    pub test_mode: bool,
    /// Generate documentation for packages
    pub generate_docs: bool,
    /// Generate ABIs for packages
    pub generate_abis: bool,
    /// Installation directory for compiled artifacts. Defaults to current directory.
    pub install_dir: ByteSliceView,
    /// Force recompilation of all packages
    pub force_recompilation: bool,
    /// Only fetch dependency repos to MOVE_HOME
    pub fetch_deps_only: bool,
    /// Skip fetching latest git dependencies
    pub skip_fetch_latest_git_deps: bool,
    /// bytecode version. set 0 to unset and to use default
    pub bytecode_version: u32,
}

impl From<InitiaCompilerBuildConfig> for BuildConfig {
    fn from(val: InitiaCompilerBuildConfig) -> Self {
        Self {
            dev_mode: val.dev_mode,
            test_mode: val.test_mode,
            generate_docs: val.generate_docs,
            generate_abis: val.generate_abis,
            install_dir: val.install_dir.into(),
            force_recompilation: val.force_recompilation,
            architecture: Some(Architecture::Move),
            fetch_deps_only: val.fetch_deps_only,
            skip_fetch_latest_git_deps: val.skip_fetch_latest_git_deps,
            compiler_config: CompilerConfig {
                bytecode_version: if val.bytecode_version == 0 {
                    None
                } else {
                    Some(val.bytecode_version)
                },
                ..Default::default()
            },
            ..Default::default()
        }
    }
}

#[repr(C)]
pub struct InitiaCompilerTestOption {
    /// Bound the amount of gas used by any one test.
    pub gas_limit: u64,
    /// A filter string to determine which unit tests to run. A unit test will be run only if it
    /// contains this string in its fully qualified (<addr>::<module_name>::<fn_name>) name.
    pub filter: ByteSliceView,
    /// List all tests
    pub list: bool,
    /// Number of threads to use for running tests.
    pub num_threads: usize,
    /// Report test statistics at the end of testing
    pub report_statistics: bool,
    /// Show the storage state at the end of execution of a failing test
    pub report_storage_on_error: bool,
    /// Ignore compiler's warning, and continue run tests
    pub ignore_compile_warnings: bool,
    /// Use the stackless bytecode interpreter to run the tests and cross check its results with
    /// the execution result from Move VM.
    pub check_stackless_vm: bool,
    /// Verbose mode
    pub verbose_mode: bool,
    /// Collect coverage information for later use with the various `package coverage` subcommands
    pub compute_coverage: bool,
}

impl From<InitiaCompilerTestOption> for Test {
    fn from(val: InitiaCompilerTestOption) -> Self {
        Self {
            gas_limit: match val.gas_limit {
                0 => None,
                _ => Some(val.gas_limit),
            },
            filter: val.filter.into(),
            list: val.list,
            num_threads: val.num_threads,
            report_statistics: val.report_statistics,
            report_storage_on_error: val.report_storage_on_error,
            ignore_compile_warnings: val.ignore_compile_warnings,
            check_stackless_vm: val.check_stackless_vm,
            verbose_mode: val.verbose_mode,
            compute_coverage: val.compute_coverage,
        }
    }
}

#[repr(C)]
pub struct InitiaCompilerProveOption {
    // Verbosity level
    pub verbosity: ByteSliceView,
    /// Filters targets out from the package. Any module with a matching file name will
    /// be a target, similar as with `cargo test`.
    pub filter: ByteSliceView,
    /// Whether to display additional information in error reports. This may help
    /// debugging but also can make verification slower.
    pub trace: bool,
    /// Whether to use cvc5 as the smt solver backend. The environment variable
    /// `CVC5_EXE` should point to the binary.
    pub cvc5: bool,
    /// The depth until which stratified functions are expanded.
    pub stratification_depth: usize,
    /// A seed for the prover.
    pub random_seed: usize,
    /// The number of cores to use for parallel processing of verification conditions.
    pub proc_cores: usize,
    /// A (soft) timeout for the solver, per verification condition, in seconds.
    pub vc_timeout: usize,
    /// Whether to check consistency of specs by injecting impossible assertions.
    pub check_inconsistency: bool,
    /// Whether to keep loops as they are and pass them on to the underlying solver.
    pub keep_loops: bool,
    /// Number of iterations to unroll loops. set 0 to unset
    pub loop_unroll: u64,
    /// Whether output for e.g. diagnosis shall be stable/redacted so it can be used in test
    /// output.
    pub stable_test_output: bool,
    /// Whether to dump intermediate step results to files.
    pub dump: bool,
    /// indicating that this prover run is for a test.
    pub for_test: bool,
}

impl From<InitiaCompilerProveOption> for ProverOptions {
    fn from(val: InitiaCompilerProveOption) -> Self {
        let verbosity_str: Option<String> = val.verbosity.into();
        let verbosity = verbosity_str.map(|s| LevelFilter::from_str(s.as_str()).unwrap());
        Self {
            verbosity,
            filter: val.filter.into(),
            trace: val.trace,
            cvc5: val.cvc5,
            stratification_depth: val.stratification_depth,
            random_seed: val.random_seed,
            proc_cores: val.proc_cores,
            vc_timeout: val.vc_timeout,
            check_inconsistency: val.check_inconsistency,
            keep_loops: val.keep_loops,
            loop_unroll: if val.loop_unroll == 0 {
                None
            } else {
                Some(val.loop_unroll)
            },
            stable_test_output: val.stable_test_output,
            dump: val.dump,
            for_test: val.for_test,
            ..Default::default()
        }
    }
}
