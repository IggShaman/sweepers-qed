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

A game run alternates between 'solver running' and 'solver suspended' states. The former executes the loop described below, and the latter allows the player to use the game normally. In that state, the player can uncover enough cells to get information for the solver to work with.

When the player uncovers a cell without a landmine, that cell is added to a queue of points of interest (POI for short).

### LP-based solver loop

1. If the POI queue is empty, stop. Otherwise, pick the top cell and look at the surrounding 13×13 grid of cells (borders permitting).

2. Create a new LP from scratch.

3. Let's use C for a grid cell that is covered and not marked as a landmine. For each C:

   - create x[C]: a floating-point variable in the [0, 1] range. 0 means there is no landmine, 1 means there is one.

4. Let's use U for a grid cell that is uncovered. For each U, let:

   - clue(U) — the number shown on U: how many landmines touch it
   - marked(U) — the number of U's neighbors already marked as landmines
   - X[U] — the set of all variables x[C] where C is U's neighbor

   Skip U if X[U] is empty, or if U is overmarked (`clue(U) < marked(U)`). Otherwise, add a linear equation to the LP:

   $$\sum_{x \,\in\, X[U]} x \;=\; \mathrm{clue}(U) - \mathrm{marked}(U)$$

   With U ranging over the 13×13 window, the variables can span a 15×15 grid.

5. For each variable x[C], use GLPK to find `min`/`max` and check:

   ```
   - max x[C] ≤ 1 − ε
     ⟹  too far from 1
     ⟹  cannot be 1
     ⟹  no landmine at C
     → uncover it
   - min x[C] ≥ ε
     ⟹  too far from 0
     ⟹  cannot be 0
     ⟹  must have a landmine at C
     → mark it
   ```

6. Add all newly resolved cells — uncovered or marked — to the queue.

7. Go back to step 1.

None of this requires anything beyond what a player knows from looking at the board.

## Correctness vs completeness

With the ε-based conditions from above, the approach soundly proves whether or not a cell holds a landmine.

Correctness depends on ε being larger than the LP solver's feasibility tolerance. But this may cost completeness: a conservative ε will discard borderline but genuine proofs. Exact rational arithmetic would address this, at a considerable performance penalty.

The 13×13 window may discard proofs that require information from further out. This can be addressed by adding all of the available information to the LP, though that can be prohibitively expensive.

I picked `ε = 1e-5` to be larger than GLPK's tolerance `tol_bnd = 1e-7`. When playing the game manually, I never looked beyond the 5×5 window, so a much larger window of 13×13 seemed like a generous choice for the neighborhood size.

I was never able to advance the game by hand further than the solver got with `ε = 1e-5` and the 13×13 window.

## Backstory

Back in the late '90s, I was a big fan of [kmines](https://apps.kde.org/kmines/) and spent weeks playing it — what a game. It was as emotionally draining and addictive as a jigsaw puzzle. Eventually I overcame the obsession and moved on with my life.

Fast forward to 2015: I ordered a new desktop, mainly to play GTA V, which had just come out for PCs. The GPU took an extra week to ship, so the desktop arrived without it. In the meantime I used an older card to test the hardware, install Linux, and so on — and found out the only game I could realistically play was — again — kmines! This time I lost three days to it and realized a way to break the spell was to write a solver.

Which I did, using GLPK and the very simple LP relaxation described above. It took another three days to implement, rendering the original game uninteresting to play.

This week-long detour was so intense I never did play GTA V.

Three years later, I was interviewing for a job at a certain company that has a quirk in its process — the first thing you did at the day-long onsite was present one of your own projects to potential teammates, in 30–45 minutes. This game fit just fine. I got the offer.

## License

[GPLv3](LICENSE)
