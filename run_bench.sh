# nix-shell --argstr unikernel ./benchmarks/test --run ./test.sh
nix-shell --argstr unikernel ./benchmarks/seq_read_bench --run ./seq_read_bench.py
# nix-shell --argstr unikernel ./benchmarks/rng_read_bench --run ./rng_read_bench.py