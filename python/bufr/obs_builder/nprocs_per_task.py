
import os

def nprocs_per_task(default=8):
    """
    Determine the number of processes to use.

    The number of processes is determined from the environment variables
    `CPUS_PER_TASK`, `SLURM_CPUS_PER_TASK`, or falls back to the default value.

    Examples from command line:
        1. Run with 1 slurm task and reserve 12 CPU cores for this one task.
           In Python, this will spawn 12 worker processes via multiprocessing.Pool.
           (SLURM_CPUS_PER_TASK = 12)
           ``srun -n 1 --cpus_per_tasks=12 python bufr_ssmis.py``

        2. Run without slurm and tell Python to spawn 12 worker processes.
           ``export CPUS_PER_TASKS=12``
           ``python bufr_ssmis.py``

    :param default: Fallback default if nothing is found. Default is 8.
    :type default: int

    :return: Number of processes to use.
    :rtype: int
    """
    env_nprocs = os.getenv("CPUS_PER_TASK")
    slurm_nprocs = os.environ.get("SLURM_CPUS_PER_TASK")

    print(f"CPUS_PER_TASK={env_nprocs}, SLURM_CPUS_PER_TASK={slurm_nprocs}")

    try:
        return int(env_nprocs or slurm_nprocs or default)
    except ValueError:
        print(f"[WARN] Invalid environment variable value. Falling back to default: {default}")
        return default



