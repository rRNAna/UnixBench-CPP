#!/usr/bin/env python3
# Standard library import for file paths,
# system commands, regular expressions, time processing, etc.
import os, resource, time, math, argparse
import subprocess, statistics, re
import shutil, psutil, sys
import concurrent.futures
from pathlib import Path
# Drawing Library
# import matplotlib.pyplot as plt
# import pandas as pd
# -----------------------------------------------------------------------------
# Configuration
# -----------------------------------------------------------------------------
VERSION     = "6.3.0"
LANGUAGE    = "en_US.utf8"
LONG_ITER   = 10
SHORT_ITER  = 3
C_COMPILER  = "clang"
#C_COMPILER  = "gcc"

# Set the working directory
BINDIR      = Path("./pgms")
RESULTDIR   = Path("./results")
TMPDIR      = Path("./tmp")
# 1) Clean and create TMPDIR
if TMPDIR.exists():
    shutil.rmtree(TMPDIR)
TMPDIR.mkdir(parents=True)

# 2) Copy the original testdir to TMPDIR/testdir
shutil.copytree(Path.cwd() / "testdir", TMPDIR / "testdir")
# ------------------------------------------------------------------------------
# BenchmarkParser: Manage the output parser for each benchmark
# ------------------------------------------------------------------------------
class BenchmarkParser:
    """
    Used to register and schedule output parsers for each benchmark
    """
    def __init__(self):
        self.parsers = {}

    # Decorator for registering parsing functions
    def register(self, name):
        def wrapper(func):
            self.parsers[name] = func
            return func
        return wrapper

    # Call the corresponding parser according to the benchmark name
    def parse(self, name, output):
        if name in self.parsers:
            return self.parsers[name](output)
        return self.default_parse(output)

    # Default parser: tries to extract COUNT| or MWIPS fields from the output
    def default_parse(self, output):
        # fallback: COUNT|... or MWIPS
        results = []
        for line in output.strip().split("\n"):
            if line.startswith("COUNT|"):
                parts = line.strip().split("|")
                if len(parts) >= 2:
                    try:
                        results.append(float(parts[1]))
                    except:
                        continue
        if results:
            return {"COUNT0": max(results)}
        m = re.search(r"^MWIPS\s+([0-9.]+)", output, re.MULTILINE)
        if m:
            return {"COUNT0": float(m.group(1))}
        return {}

# ------------------------------------------------------------------------------
# Benchmark: Represents a benchmark test item
# ------------------------------------------------------------------------------
class Benchmark:
    """
    Represents a single benchmark test item
    """
    def __init__(self, name, msg, command, parser: BenchmarkParser, verbose=False):
        self.name = name        # Test Name
        self.msg = msg          # Display Name
        self.command = command  # Command Line Parameters
        self.parser = parser
        self.samples = []       # Store the results of each run
        self.verbose = verbose  # Test output verbosity

    # def run_once(self, concurrency=1, logdir=None, report_mode='html'):
    #     processes = []
    #     outputs = []
    #     start = time.time()
    #     cwd = str(TMPDIR / "testdir") if self.name in {"shell1", "shell8"} else None
    #
    #     # 不绑核的 benchmark
    #     no_affinity_list = {"dhry_reg", "fstime-w", "fstime-r", "fstime"}
    #     bind_affinity = self.name not in no_affinity_list
    #
    #     for thread_id in range(concurrency):
    #         # 对 fstime/fsbuffer/fsdisk 系列使用 thread-specific TMPDIR
    #         if self.name.startswith("fstime") or self.name.startswith("fsbuffer") or self.name.startswith("fsdisk"):
    #             thread_tmp_dir = TMPDIR / f"testdir/thread-{thread_id}"
    #             thread_tmp_dir.mkdir(parents=True, exist_ok=True)
    #             cmd = [arg if arg != str(TMPDIR) else str(thread_tmp_dir) for arg in self.command]
    #         else:
    #             cmd = self.command[:]
    #         try:
    #             proc = subprocess.Popen(
    #                 cmd,
    #                 stdout=subprocess.PIPE,
    #                 stderr=subprocess.PIPE,
    #                 text=True,
    #                 cwd=cwd,
    #                 start_new_session=True
    #             )
    #         except OSError as e:
    #             print(f"[ERROR] Failed to launch subprocess: {e}", file=sys.stderr)
    #             print(f"[DEBUG] Command attempted: {args}", file=sys.stderr)
    #             raise
    #
    #         if bind_affinity:
    #             try:
    #                 p = psutil.Process(proc.pid)
    #                 cpu_id = thread_id % os.cpu_count()
    #                 p.cpu_affinity([cpu_id])
    #             except Exception as e:
    #                 print(f"[WARN] Failed to set affinity for {self.name}, pid={proc.pid}: {e}")
    #
    #         processes.append((proc, time.time()))
    #
    #     thread_times = []
    #     with concurrent.futures.ThreadPoolExecutor(max_workers=len(processes)) as executor:
    #         futures = [executor.submit(proc.communicate) for proc, _ in processes]
    #         for i, future in enumerate(futures):
    #             stdout, stderr = future.result()
    #             end_time = time.time()
    #             outputs.append(stdout + stderr)
    #             thread_times.append(end_time - processes[i][1])
    #
    #     avg_elapsed = sum(thread_times) / len(thread_times)
    #     combined_output = "\n".join(outputs)
    #
    #     if self.verbose:
    #         print(f"\n-------------------------------------{self.name}-----------------")
    #         print(combined_output.strip())
    #         print(f"\n-------------------------------------END-----------------")
    #
    #     if logdir and report_mode in ("all", "log"):
    #         (logdir / f"{self.name}.log").write_text(combined_output)
    #     if logdir and report_mode in ("all",):
    #         (logdir / f"{self.name}.txt").write_text(f"CMD: {' '.join(self.command)}\n\n{combined_output}")
    #
    #     # 解析每个子进程输出
    #     for output in outputs:
    #         result = self.parser.parse(self.name, output)
    #         if result and "COUNT0" in result:
    #             result["avg_elapsed"] = avg_elapsed
    #             if self.name in {
    #                 "dhry_reg", "whetstone-double", "pipe", "context1", "syscall", "sysexec"
    #             }:
    #                 result["COUNT1"] = 10
    #             elif self.name in {
    #                 "execl", "spawn", "fstime", "fstime-w", "fstime-r",
    #                 "fsbuffer", "fsbuffer-w", "fsbuffer-r",
    #                 "fsdisk", "fsdisk-w", "fsdisk-r"
    #             }:
    #                 result["COUNT1"] = 20
    #             elif self.name in {"shell1", "shell8"}:
    #                 result["COUNT1"] = 60
    #             elif self.name.startswith("2d-") or self.name in {"ubgears"}:
    #                 result["COUNT1"] = 3 if self.name.startswith("2d-") else 20
    #             elif self.name in {"grep"}:
    #                 result["COUNT1"] = 30
    #             else:
    #                 result["COUNT1"] = 10  # fallback
    #             self.samples.append(result)

    def run_once(self, concurrency=1, logdir=None, report_mode='html'):
        processes = []
        outputs = []
        start = time.time()
        cwd = str(TMPDIR / "testdir") if self.name in {"shell1", "shell8"} else None

        no_affinity_list = {"dhry_reg", "fstime-w", "fstime-r", "fstime"}
        bind_affinity = self.name not in no_affinity_list

        # 启动子进程
        for thread_id in range(concurrency):
            if self.name.startswith(("fstime", "fsbuffer", "fsdisk")):
                thread_tmp_dir = TMPDIR / f"testdir/thread-{thread_id}"
                thread_tmp_dir.mkdir(parents=True, exist_ok=True)
                cmd = [arg if arg != str(TMPDIR) else str(thread_tmp_dir) for arg in self.command]
            else:
                cmd = self.command[:]

            try:
                needs_tty = (self.name == "whetstone-double")
                stdin_arg = None if needs_tty else subprocess.DEVNULL
                proc = subprocess.Popen(
                    cmd,
                    stdout=subprocess.PIPE,
                    stderr=subprocess.STDOUT,
                    stdin=stdin_arg,
                    text=True, bufsize=1,
                    encoding="utf-8", errors="replace",
                    cwd=cwd, start_new_session=True
                )
                # proc = subprocess.Popen(
                #     cmd,
                #     stdout=subprocess.PIPE,  # 只开一条管道
                #     stderr=subprocess.STDOUT,  # 合并 stderr，避免回压
                #     stdin=subprocess.DEVNULL,  # 明确不接收输入，防“等待输入”
                #     text=True,
                #     bufsize=1,  # 行缓冲（配合 text=True）
                #     cwd=cwd,
                #     start_new_session=True  # 新进程组，便于整体清理
                # )
            except OSError as e:
                print(f"[ERROR] Failed to launch subprocess: {e}", file=sys.stderr)
                print(f"[DEBUG] Command attempted: {cmd}", file=sys.stderr)
                raise

            if bind_affinity:
                try:
                    p = psutil.Process(proc.pid)
                    cpu_id = thread_id % os.cpu_count()
                    p.cpu_affinity([cpu_id])
                except Exception as e:
                    print(f"[WARN] Failed to set affinity for {self.name}, pid={proc.pid}: {e}")

            processes.append((proc, time.time()))

        # 实时消费输出（逐行），并给每个子进程设置超时
        thread_times = [None] * len(processes)
        per_proc_timeout = 60 * 60  # 每个子进程 1 小时上限，可按需调

        def _drain(proc, idx, sink_lines):
            deadline = time.time() + per_proc_timeout
            try:
                for line in proc.stdout:
                    sink_lines.append(line)
                    # 可在此解析/打印进度
                    if time.time() > deadline:
                        raise subprocess.TimeoutExpired(proc.args, per_proc_timeout)
                ret = proc.wait(timeout=max(0, deadline - time.time()))
                end_time = time.time()
                thread_times[idx] = end_time - processes[idx][1]
                return ret
            except subprocess.TimeoutExpired:
                # 超时则杀掉整个进程组
                try:
                    os.killpg(proc.pid, signal.SIGTERM)
                    time.sleep(2)
                    if proc.poll() is None:
                        os.killpg(proc.pid, signal.SIGKILL)
                finally:
                    thread_times[idx] = time.time() - processes[idx][1]
                return None

        # 为每个子进程单独开一个读取线程（数量=并发），边读边防回压
        sink_list = [[] for _ in processes]
        with concurrent.futures.ThreadPoolExecutor(max_workers=len(processes)) as ex:
            futs = [ex.submit(_drain, proc, i, sink_list[i]) for i, (proc, _) in enumerate(processes)]
            for _ in concurrent.futures.as_completed(futs):
                pass

        # 汇总输出
        outputs = ["".join(sink) for sink in sink_list]
        avg_elapsed = sum(t for t in thread_times if t is not None) / max(1, len([t for t in thread_times if
                                                                                  t is not None]))
        combined_output = "\n".join(outputs)

        if self.verbose:
            print(f"\n-------------------------------------{self.name}-----------------")
            print(combined_output.strip())
            print(f"\n-------------------------------------END-----------------")

        if logdir and report_mode in ("all", "log"):
            (logdir / f"{self.name}.log").write_text(combined_output)
        if logdir and report_mode in ("all",):
            (logdir / f"{self.name}.txt").write_text(f"CMD: {' '.join(self.command)}\n\n{combined_output}")

        # 解析每个子进程输出
        for output in outputs:
            result = self.parser.parse(self.name, output)
            if result and "COUNT0" in result:
                result["avg_elapsed"] = avg_elapsed
                if self.name in {"dhry_reg", "whetstone-double", "pipe", "context1", "syscall", "sysexec"}:
                    result["COUNT1"] = 10
                elif self.name in {"execl", "spawn", "fstime", "fstime-w", "fstime-r",
                                   "fsbuffer", "fsbuffer-w", "fsbuffer-r",
                                   "fsdisk", "fsdisk-w", "fsdisk-r"}:
                    result["COUNT1"] = 20
                elif self.name in {"shell1", "shell8"}:
                    result["COUNT1"] = 60
                elif self.name.startswith("2d-") or self.name in {"ubgears"}:
                    result["COUNT1"] = 3 if self.name.startswith("2d-") else 20
                elif self.name in {"grep"}:
                    result["COUNT1"] = 30
                else:
                    result["COUNT1"] = 10
                self.samples.append(result)
                
    # Repeat the Benchmark multiple times
    def run(self, times, concurrency, logdir, report_mode):
        print(f"[{self.name:<10}] ", end="", flush=True)
        for i in range(times):
            try:
                self.run_once(concurrency, logdir, report_mode)
                print(f"✔({i + 1}/{times}) ", end="", flush=True)
            except Exception as e:
                print(f"✘({i + 1}/{times}) ", end="", flush=True)
        print("")  # 换行，完成一项 benchmark 的输出

    # Count the results (mixed strategy consistent with original UnixBench)
    def summarize(self):
        if not self.samples:
            return 0.0, 0

        # For syscall class test: total count0 / average count1 (multi-threaded)
        if self.name in {"syscall", "pipe", "context1", "spawn", "execl"}:
            total_count = sum(r["COUNT0"] for r in self.samples if "COUNT0" in r)
            avg_count1 = statistics.mean(r["COUNT1"] for r in self.samples if "COUNT1" in r)
            return total_count / avg_count1 if avg_count1 else 0.0, len(self.samples)

        # Same as original UnixBench: dhry_reg, whetstone-double, etc. use sums
        if self.name in {"dhry_reg", "whetstone-double", "shell1", "shell8", "shell16", "fstime-w", "fstime-r", "fstime"}:
            values = [r["COUNT0"] for r in self.samples if "COUNT0" in r]
            return sum(values), len(values)

        # After discarding the lowest value in the first 1/3, calculate the geometric mean
        values = [r["COUNT0"] for r in self.samples if "COUNT0" in r]
        values.sort()
        trimmed = values[len(values)//3:]  # Discard the lowest value in the first 1/3
        if not trimmed:
            return 0.0, 0
        geo = math.exp(sum(math.log(v) for v in trimmed) / len(trimmed))
        return geo, len(trimmed)

    # Get the last score
    def get_last_score(self):
        if not self.samples:
            return 0.0
        return self.samples[-1]["COUNT0"]

# ------------------------------------------------------------------------------
# BenchmarkSuite: Manage all tests
# ------------------------------------------------------------------------------
class BenchmarkSuite:
    """
    Manage the registration, execution and scoring of all benchmarks
    """
    def __init__(self, verbose = False):
        self.parser = BenchmarkParser()
        self.benchmarks = {}
        self.verbose = verbose

        # The baseline value of each benchmark is used to calculate the Index Score
        self.baselines = {
            "dhry_reg":         116700.0,
            "whetstone-double": 55.0,
            "execl":            43.0,
            "fstime-w":         3960.0,
            "fstime-r":         1655.0,
            "fstime":           5800.0,
            "pipe":             12440.0,
            "context1":         4000.0,
            "spawn":            126.0,
            "shell1":           42.4,
            "shell8":           6.0,
            "syscall":          15000.0
        }
    # Add a benchmark
    def add(self, name, msg, command):
        self.benchmarks[name] = Benchmark(name, msg, command, self.parser, verbose=self.verbose)

    # Register the parser (externally called through the decorator)
    def register_parser(self, name):
        return self.parser.register(name)

    # Execute all or specified benchmarks
    def run(self, selected=None, repeat=None, concurrency=1, logdir=None, report_mode='html'):
        selected = selected or list(self.baselines.keys())
        final = []
        self.index_values = []

        # Define a list of benchmark names for "short_tests"
        short_tests = {"execl", "spawn", "fstime", "fstime-w", "fstime-r",
                       "fsbuffer", "fsbuffer-w", "fsbuffer-r",
                       "fsdisk", "fsdisk-w", "fsdisk-r",
                       "shell1", "shell8"}

        unknown_benchmarks = False
        for name in selected:
            if name not in self.benchmarks:
                print(f"❌ Unknown benchmark: {name}")
                print("✅ Available benchmarks are:\n")
                for bname, bench in self.benchmarks.items():
                    print(f"  {bname:<20} {bench.msg}")
                unknown_benchmarks = True
                continue
            bench = self.benchmarks[name]

            # If the user does not specify repeat, the default rule is used.
            if repeat is None:
                # Automatically select the number of iterations based on the benchmark type
                times = 3 if name in short_tests else 10
            else:
                times = repeat

            bench.run(times, concurrency, logdir, report_mode)

            score, count = bench.summarize()
            baseline = self.baselines.get(name, 0.0)
            index = score / baseline * 10 if baseline else 0.0
            if index > 0:
                self.index_values.append(index)
            final.append((name, bench.msg, score, baseline, index, count))
        if unknown_benchmarks:
            return []
        return final

    def report(self, results):
        print("\n--- Summary ---")
        print(f"{'Benchmark':<42} {'Baseline':>10} {'Result':>14} {'Index':>12} {'Samples':>8}")
        print("-" * 90)
        for name, msg, score, baseline, index, count in results:
            print(f"{msg:<42} {baseline:>10.2f} {score:>14.2f} {index:>12.2f} {count:>8d}")


# ------------------------------------------------------------------------------
# Generate benchmark results image
# ------------------------------------------------------------------------------
# def generate_benchmark_plot(results, output_path):
#     data = {
#         "Benchmark": [msg for _, msg, _, _, _, _ in results],
#         "Result": [score for _, _, score, _, _, _ in results]
#     }
#     df = pd.DataFrame(data)
#     plt.figure(figsize=(12, 6))
#     bars = plt.barh(df["Benchmark"], df["Result"], edgecolor='black')
#     plt.xlabel("Performance Score")
#     plt.title("UnixBench Benchmark Results")
#     plt.grid(True, axis='x', linestyle='--', alpha=0.7)
#     for bar in bars:
#         width = bar.get_width()
#         plt.text(width * 1.01, bar.get_y() + bar.get_height()/2,
#                  f"{width:,.0f}", va='center')
#     plt.tight_layout()
#     plt.savefig(output_path)
#     plt.close()
class OS_Tuning:

    def __init__(self):
        self.cores = os.cpu_count()

        self.REQUIRED_NOFILE = max(65536, 4096 + 8 * int(os.getenv("UB_CONCURRENCY", self.cores)))  # Estimated by concurrency

        self.TARGET_NOFILE   = min(1048576, self.REQUIRED_NOFILE)  # The upper limit is 1,048,576, which is sufficient and safe

    def set_nofile_or_explain(target):
        soft, hard = resource.getrlimit(resource.RLIMIT_NOFILE)
        if soft >= target:
            return soft, hard, soft  # Satisfied
        # Try raising the soft cap to min(target, hard)
        new_soft = min(target, hard)
        try:
            resource.setrlimit(resource.RLIMIT_NOFILE, (new_soft, hard))
            return soft, hard, new_soft
        except Exception as e:
            # Provide clear guidance
            print(f"[FATAL] open files (nofile) Insufficient soft cap：now soft={soft}, hard={hard}, needs >= {target}")
            print("Repair method (execute and run again in the same shell):")
            print(f"  ulimit -n {target}")
            print("If the hard is not enough, the administrator needs to increase it:")
            print(f"  sudo bash -c 'echo \"* soft nofile {target}\\n* hard nofile {max(target, hard)}\" > /etc/security/limits.d/90-nofile.conf'")
            print("  Relogin or add to systemd service: LimitNOFILE=" + str(target))
            sys.exit(1)

    def raise_fd_limit(min_limit=4096):
        soft, hard = resource.getrlimit(resource.RLIMIT_NOFILE)
        if soft < min_limit:
            try:
                new_soft = min(min_limit, hard)
                resource.setrlimit(resource.RLIMIT_NOFILE, (new_soft, hard))
                print(f"[INFO] Raised file descriptor limit to {new_soft}")
            except Exception as e:
                print(f"[WARN] Unable to raise file descriptor limit: {e}", file=sys.stderr)
# ------------------------------------------------------------------------------
# Main program
# ------------------------------------------------------------------------------
def main():
    # Print UnixBench Logo
    with open("pgms/unixbench.logo") as f:
        print(f.read())

    # Set the system file descriptor
    ost = OS_Tuning()
    TARGET_NOFILE = ost.TARGET_NOFILE
    OS_Tuning.raise_fd_limit(65536)
    old_soft, old_hard, new_soft = OS_Tuning.set_nofile_or_explain(TARGET_NOFILE)

    # Parsing command line arguments
    parser = argparse.ArgumentParser(
        description="UnixBench Python 测试框架\n用法示例:\n  ./Run.py dhry_reg whetstone-double -i 5",
        formatter_class=argparse.RawTextHelpFormatter
    )
    parser.add_argument("--list", action="store_true", help="列出所有可用的 benchmark 项")
    parser.add_argument("-c", "--concurrency", type=int, help="指定测试并发数（可重复多次）例如 -c 1 -c 16")
    parser.add_argument("tests", nargs="*", help="指定要运行的测试项")
    parser.add_argument("-i", "--repeat", type=int, default=None, help="重复次数（不指定则自动按 benchmark 类型判断）")
    parser.add_argument("-v", "--verbose", action="store_true", help="详细输出每个 benchmark 的执行信息")
    parser.add_argument("--report", choices=["all", "html", "log"], default="html", help="指定输出报告类型: html（默认），log，仅文本或 all")

    args = parser.parse_args()

    os.makedirs(RESULTDIR, exist_ok=True) # Make sure the output directory exists

    from datetime import datetime
    timestamp = datetime.now().strftime("%Y%m%d-%H%M%S")
    logdir = RESULTDIR / f"run-{timestamp}"
    logdir.mkdir(parents=True, exist_ok=True)

    suite = BenchmarkSuite(verbose=args.verbose)

    # Add benchmark definition
    ##########################
    ## System Benchmarks    ##
    ##########################
    # Command composition and purpose of each test item
    suite.add ("dhry_reg", "Dhrystone 2 using register variables", [str(BINDIR / "dhry_reg"), "10"])
    suite.add("whetstone-double", "Double-Precision Whetstone", [str(BINDIR / "whetstone-double")])
    suite.add ("syscall", "System Call Overhead", [str(BINDIR / "syscall"), "10"])
    suite.add("context1", "Pipe-based Context Switching", [str(BINDIR / "context1"), "10"])
    suite.add("pipe", "Pipe Throughput", [str(BINDIR / "pipe"), "10"])
    suite.add("spawn", "Process Creation", [str(BINDIR / "spawn"), "30"])
    suite.add("execl", "Execl Throughput", [str(BINDIR / "execl"), "30"])
    suite.add("fstime-w", "File Write 1024 bufsize 2000 maxblocks",
     [str(BINDIR / "fstime"), "-w", "-t", "30", "-d", str(TMPDIR), "-b", "1024", "-m", "2000"])
    suite.add("fstime-r", "File Read 1024 bufsize 2000 maxblocks",
     [str(BINDIR / "fstime"), "-r", "-t", "30", "-d", str(TMPDIR), "-b", "1024", "-m", "2000"])
    suite.add("fstime", "File Copy 1024 bufsize 2000 maxblocks",
     [str(BINDIR / "fstime"), "-c", "-t", "30", "-d", str(TMPDIR), "-b", "1024", "-m", "2000"])
    suite.add("fsbuffer-w", "File Write 256 bufsize 500 maxblocks",
     [str(BINDIR / "fstime"), "-w", "-t", "30", "-d", str(TMPDIR), "-b", "256", "-m", "500"])
    suite.add("fsbuffer-r", "File Read 256 bufsize 500 maxblocks",
     [str(BINDIR / "fstime"), "-r", "-t", "30", "-d", str(TMPDIR), "-b", "256", "-m", "500"])
    suite.add("fsbuffer", "File Copy 256 bufsize 500 maxblocks",
     [str(BINDIR / "fstime"), "-c", "-t", "30", "-d", str(TMPDIR), "-b", "256", "-m", "500"])
    suite.add("fsdisk-w", "File Write 4096 bufsize 8000 maxblocks",
     [str(BINDIR / "fstime"), "-w", "-t", "30", "-d", str(TMPDIR), "-b", "4096", "-m", "8000"])
    suite.add("fsdisk-r", "File Read 4096 bufsize 8000 maxblocks",
     [str(BINDIR / "fstime"), "-r", "-t", "30", "-d", str(TMPDIR), "-b", "4096", "-m", "8000"])
    suite.add("fsdisk", "File Copy 4096 bufsize 8000 maxblocks",
     [str(BINDIR / "fstime"), "-c", "-t", "30", "-d", str(TMPDIR), "-b", "4096", "-m", "8000"])
    suite.add("shell1", "Shell Scripts (1 concurrent)", [os.path.abspath(BINDIR / "looper"), "60", os.path.abspath(BINDIR / "multi.sh"), "1"])
    suite.add("shell8", "Shell Scripts (8 concurrent)", [os.path.abspath(BINDIR / "looper"), "60", os.path.abspath(BINDIR / "multi.sh"), "8"])
    suite.add("shell16", "Shell Scripts (16 concurrent)", [str(BINDIR / "looper"), "60", str(BINDIR / "multi.sh"), "16"])
    ##########################
    ## Graphics Benchmarks  ##
    ##########################
    suite.add("2d-rects", "2D graphics: rectangles", [str(BINDIR / "gfx-x11"), "rects", "3", "2"])
    suite.add("2d-lines", "2D graphics: lines", [str(BINDIR / "gfx-x11"), "lines", "3", "2"])
    suite.add("2d-circle", "2D graphics: circles", [str(BINDIR / "gfx-x11"), "circle", "3", "2"])
    suite.add("2d-ellipse", "2D graphics: ellipses", [str(BINDIR / "gfx-x11"), "ellipse", "3", "2"])
    suite.add("2d-shapes", "2D graphics: polygons", [str(BINDIR / "gfx-x11"), "shapes", "3", "2"])
    suite.add("2d-aashapes", "2D graphics: aa polygons", [str(BINDIR / "gfx-x11"), "aashapes", "3", "2"])
    suite.add("2d-polys", "2D graphics: complex polygons", [str(BINDIR / "gfx-x11"), "polys", "3", "2"])
    suite.add("2d-text", "2D graphics: text", [str(BINDIR / "gfx-x11"), "text", "3", "2"])
    suite.add("2d-blit", "2D graphics: images and blits", [str(BINDIR / "gfx-x11"), "blit", "3", "2"])
    suite.add("2d-window", "2D graphics: windows", [str(BINDIR / "gfx-x11"), "window", "3", "2"])
    suite.add("ubgears", "3D graphics: gears", [str(BINDIR / "ubgears"), "-time", "20", "-v"])
    ##########################
    ## Non-Index Benchmarks ##
    ##########################
    suite.add("C", f"C Compiler Throughput ({C_COMPILER})", [str(BINDIR / "looper"), "60", C_COMPILER, "cctest.c"])
    suite.add("arithoh", "Arithoh", [str(BINDIR / "arithoh"), "10"])
    suite.add("short", "Arithmetic Test (short)", [str(BINDIR / "short"), "10"])
    suite.add("int", "Arithmetic Test (int)", [str(BINDIR / "int"), "10"])
    suite.add ("long", "Arithmetic Test (long)", [str(BINDIR / "long"), "10"])
    suite.add("float", "Arithmetic Test (float)", [str(BINDIR / "float"), "10"])
    suite.add("double", "Arithmetic Test (double)", [str(BINDIR / "double"), "10"])
    suite.add("dc", "Dc: sqrt(2) to 99 decimal places", [str(BINDIR / "looper"), "30", "dc"])
    suite.add("hanoi", "Recursion Test -- Tower of Hanoi", [str(BINDIR / "hanoi"), "20"])
    suite.add("grep", "Grep a large file", [str(BINDIR / "looper"), "30", "grep", "-c", "gimp", "large.txt"])
    suite.add("sysexec", "Exec System Call Overhead", [str(BINDIR / "syscall"), "10", "exec"])

    # Register a specific benchmark output parser
    @suite.register_parser("dhry_reg")
    def parse_dhry_reg(output):
        for line in output.splitlines():
            if line.startswith("COUNT|"):
                parts = line.split("|")
                return {"COUNT0": float(parts[1])}
        return {}

    @suite.register_parser("whetstone-double")
    def parse_whets(output):
        # 1) 先抓 MWIPS 行（行首有 "MWIPS"）
        m = re.search(r"(?m)^MWIPS\s+([0-9]+(?:[.,][0-9]+)?)", output)
        if m:
            val = m.group(1).replace(',', '.')
            return {"COUNT0": float(val)}

        # 2) 兼容 COUNT 行：COUNT|<mwips>|...|mwips
        m2 = re.search(r"(?mi)^COUNT\|([0-9]+(?:[.,][0-9]+)?)\|.*\|mwips", output)
        if m2:
            val = m2.group(1).replace(',', '.')
            return {"COUNT0": float(val)}

        # 3) （可选）在 verbose 下把最后几行打出来，便于排查
        if args.verbose:   # 注意这里用 args.verbose，而不是 self.verbose
            tail = "\n".join(output.strip().splitlines()[-20:])
            print(f"[DEBUG] whetstone-double: no MWIPS/COUNT match. Tail:\n{tail}")
        return {}

    # List all benchmark logic
    if args.list:
        print("可用的 benchmark 项如下：\n")
        for name, bench in suite.benchmarks.items():
            print(f"{name:<20} {bench.msg}")
        return

    # Concurrent List
    copies = [args.concurrency] if args.concurrency else []

    # Set the number of concurrent connections
    if not copies:
        # If -c is not specified, the default is to perform 1 and n core thread tests.
        copies = [1]
        cpu_count = os.cpu_count()
        if cpu_count and cpu_count > 1:
            copies.append(cpu_count)

    # Executing the test
    for c in copies:
        print(f"\n🧪 Run with {c} thread(s)")
        results = suite.run(args.tests, repeat=args.repeat if args.repeat else None, concurrency=c, logdir=logdir, report_mode=args.report)
        if not results:
            continue  # If it is an invalid benchmark, skip subsequent processing
        suite.report(results)  # Output

    # Generate Image Report
    # if args.report in ("all", "html"):
    #     img_path = logdir / "results.png"
    #     generate_benchmark_plot(results, img_path)


    # Calculate the geometric mean
    from statistics import geometric_mean
    valid = [i for _, _, _, _, i, _ in results if i > 0]
    if valid:
        print(f"\nSystem Benchmarks Index Score: {geometric_mean(valid):.1f}")

    if args.report in ("all", "html") and results:
        html_path = logdir / "results.html"
        with open(html_path, "w") as f:
            f.write("<html><head><title>UnixBench Report</title></head><body>")
            f.write("<h2>UnixBench Results Summary</h2>")
            f.write("<table border='1'><tr><th>Benchmark</th><th>Baseline</th><th>Result</th><th>Index</th><th>Samples</th></tr>")
            for name, msg, score, baseline, index, count in results:
                f.write(f"<tr><td>{msg}</td><td>{baseline:.1f}</td><td>{score:.1f}</td><td>{index:.1f}</td><td>{count}</td></tr>")
            f.write("</table>")
            # if args.report in ("all", "html"):
            #     f.write(f"<h3>Performance Chart</h3><img src='results.png' width='800'>")
            f.write("</body></html>")
        print(f"\nHTML 报告已生成: {html_path}")

    # Restore to the soft cap when entering the script
    resource.setrlimit(resource.RLIMIT_NOFILE, (old_soft, old_hard))


if __name__ == "__main__":
    main()
