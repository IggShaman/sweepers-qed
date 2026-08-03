# Sweeper's QED ∎

This is a simple Qt6-based landmine-sweeping game with an LP-based solver.

The 'QED' part: the solver never guesses. Every cell it uncovers has been proven safe, and every cell it marks has been proven to contain a landmine.

![Sweeper's QED solving a board](docs/game_screenshot.png)

## Build/run

```sh
# Make sure qt6 and glpk are available, on Ubuntu it'll be something like this:
sudo apt install build-essential libglpk-dev qt6-base-dev cmake

# build with:
cmake -S . -B build
cmake --build build -j $(nproc)
build/sweepers-qed

# run tests with:
ctest --test-dir build
```

## How the LP solver works

### Global picture

1. Let's call a cell a 'frontier cell' if it is covered (i.e. still in its original state), and not marked by us as having a landmine, but at the same time it has at least one neighbor cell that is uncovered, i.e. we know it does not have a landmine, and we know its neighboring landmines count.
2. A 'frontier' is the set of all frontier cells. This changes as we uncover cells during the game's run.
3. A game run alternates between 'solver running' and 'solver suspended' states. The former executes the loop described below, and the latter allows the player to use the game normally. This is the state where the player can uncover enough cells to get information for the solver to work with.

### LP-based solver loop

1. Copy the current frontier into a queue. Repeat steps 2–6 until the queue is empty.
2. Pick the top queue cell C and look at the surrounding 13×13 grid of cells (borders permitting). Pick covered and uncovered cells from the grid.
3. For each covered grid cell, create a floating-point variable x[r,c], in the [0, 1] range. 0 means there is no landmine, 1 means there is one.
4. For each uncovered grid cell G, add a linear equation: sum({G's neighboring covered cells}) = <G's number of landmines around it> − <number of G's neighboring cells marked as landmines>. As a result, the variables span a 15×15 grid.
5. For each variable: `maximize(x[r,c])` — if the maximum is ≤ 1 − ε, then the cell at (r,c) cannot have a landmine → we uncover it. `minimize(x[r,c])` — if the minimum is ≥ ε, then it can't be empty, so it must have a landmine, and we mark it as such.
6. Add all newly resolved cells — uncovered or marked — to the queue.

None of this requires anything beyond what a player knows from looking at the board.

## Correctness vs completeness

With the ε-based conditions from above, the approach gives us a sound proof of whether a landmine is there or not.

Correctness depends on ε being larger than the LP solver's feasibility tolerance. But this may cost completeness: a conservative ε will discard borderline but genuine proofs. Exact rational arithmetic would address this, at a considerable performance penalty.

The 13×13 window discards proofs that require information from further out. This can be addressed by adding all of the available information to the LP, though that can be prohibitively expensive.

I picked ε = 1e-5 to be 100× larger than GLPK's tolerance `tol_bnd`. When playing the game manually, I never looked beyond the 5×5 window, so a much larger window of 13×13 seemed like a good pick for the neighborhood size.

Empirically, ε with the 13×13 window was enough to advance the game as much as is logically possible.

## Backstory

Back in the late '90s I was a big fan of [kmines](https://apps.kde.org/kmines/) and spent weeks playing it — what a game. It was as emotionally draining and addictive as doing jigsaw puzzles. Eventually I was able to overcome the obsession and moved on with my life.

Fast forward to 2015, I ordered myself a new desktop with the main goal of playing GTA V (which just came out for PCs). The discrete graphics card took some time to procure, so the desktop arrived one week sooner without one; I used an older graphics card to boot it up, install Linux, etc., and realized that the only game I could realistically play was — again — kmines! This time I lost three days to it, and realized that a way to break the newfound addictive spell was to write a solver for it.

Which I did, using GLPK and a very simple LP relaxation described above. It took another three days to implement, rendering the original game uninteresting to play.

This week-long detour was so intense I ended up never playing GTA V.

Three years later, I was interviewing for a certain company that has a quirk in its interview process — the first thing you had to do during a day-long onsite was to present a project of yours to potential teammates, in 30–45 minutes, and this game fit just fine. I ended up getting an offer.

## License

[GPLv3](LICENSE)
