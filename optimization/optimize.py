import random

import optuna

import metrics
from album_store import AlbumStore
from dom_color import HueHistogram, SVHistogram
from run_dom_color import LoadedAlbum, errors_of, evaluate, load_albums, summary

N_TRIALS = 1000
VALIDATION_FRACTION = 0.25
SEED = 0
DEFAULT_PARAMS = {"alpha": 3, "beta": 23, "gamma": 0}


def split(albums: list[LoadedAlbum]) -> tuple[list[LoadedAlbum], list[LoadedAlbum]]:
    """Deterministic train/validation split."""
    shuffled = albums[:]
    random.Random(SEED).shuffle(shuffled)
    n_validation = round(len(shuffled) * VALIDATION_FRACTION)
    return shuffled[n_validation:], shuffled[:n_validation]


def suggest_params(trial: optuna.Trial) -> dict:
    # Add new dom_color parameters here.
    return {
        "alpha": trial.suggest_int("alpha", 0, 255),
        "beta": trial.suggest_int("beta", 0, 255),
        # Contrast in tenths; above 210 (21:1) even black is dropped.
        "gamma": trial.suggest_int("gamma", 0, 210),
    }


def make_histograms(params: dict) -> tuple[HueHistogram, SVHistogram]:
    return HueHistogram(**params), SVHistogram()


def loss_on(albums: list[LoadedAlbum], params: dict) -> float:
    return metrics.loss(errors_of(evaluate(albums, *make_histograms(params))))


def report(name: str, albums: list[LoadedAlbum], params: dict):
    print(f"  {name:<10} {summary(errors_of(evaluate(albums, *make_histograms(params))))}")


def main():
    train, validation = split(load_albums(AlbumStore()))
    print(f"{len(train)} train / {len(validation)} validation albums")

    optuna.logging.set_verbosity(optuna.logging.WARNING)
    study = optuna.create_study(direction="minimize",
                                sampler=optuna.samplers.TPESampler(seed=SEED))
    study.enqueue_trial(DEFAULT_PARAMS)  # the current defaults as a baseline
    study.optimize(lambda trial: loss_on(train, suggest_params(trial)),
                   n_trials=N_TRIALS, show_progress_bar=True)

    for label, params in [("default", DEFAULT_PARAMS), ("best", study.best_params)]:
        print(f"{label} {params}")
        report("train", train, params)
        report("validation", validation, params)


if __name__ == "__main__":
    main()
