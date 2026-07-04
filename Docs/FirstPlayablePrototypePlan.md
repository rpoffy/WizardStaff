# Wizard's Staff First Playable Prototype Plan

## Current Project Inspection

Inspection date: 2026-06-26

The workspace currently contains only Git metadata:

- `.git/`

No Unreal project scaffold was found yet:

- No `.uproject`
- No `Source/`
- No `Content/`
- No `Config/`
- No existing design notes or gameplay modules

Because there is not yet an Unreal project structure in this workspace, this plan starts with safe project scaffolding assumptions. Gameplay implementation should wait until the Unreal project files exist and the intended module names are confirmed.

## Prototype Principles

- Local multiplayer only.
- Use simple placeholder meshes, materials, collisions, and UI.
- Prefer clean C++ gameplay systems with Blueprint-exposed tuning values.
- Keep each milestone small, testable, and easy to revert.
- Avoid online multiplayer, persistence, progression systems, polished art, and broad architecture redesign.
- Build vertical slices that can be tested in-editor with local players.

## Milestone 1: Local Player Setup

Goal: Establish a minimal local multiplayer test harness.

Planned work:

- Create or confirm the Unreal project scaffold.
- Add a prototype GameMode for local play.
- Support spawning 2-4 local players.
- Assign each player a distinct PlayerStart or fallback spawn transform.
- Add simple local input mappings for movement, camera, drink/collect, and bonk.
- Expose player count and spawn spacing to Blueprint or defaults.

Validation:

- PIE can launch with multiple local players.
- Each player receives independent input.
- Players spawn without overlapping.

Notes:

- Do not add networking or online session code.
- Keep input setup compatible with Unreal's Enhanced Input if the project uses it.

## Milestone 2: Wizard Pawn / Prototype Movement

Goal: Make each wizard controllable and readable as a physical party-game character.

Planned work:

- Add a wizard pawn or character class.
- Use a capsule/root collision component and placeholder mesh.
- Implement basic movement, turning, jump or hop if useful, and simple camera behavior.
- Add Blueprint-tunable movement speed, acceleration, turn rate, and friction.
- Add a visible staff root attachment point on the wizard.

Validation:

- Multiple players can move around a test map.
- Movement feels simple and stable before staff physics is added.
- Placeholder meshes make player direction and staff position obvious.

Notes:

- Favor prototype responsiveness over animation polish.
- Avoid advanced locomotion systems until the staff gameplay needs them.

## Milestone 3: Socket-Based Staff Segment System

Goal: Build staffs from mug/segment actors attached through sockets.

Planned work:

- Add a staff component or actor owned by each wizard.
- Represent each mug/staff piece as a segment actor or component with a top socket and bottom socket.
- Attach new empty mugs to the current top socket.
- Track ordered segment data in C++.
- Expose segment count, segment mesh, socket names, scale, and spacing to Blueprint.
- Add debug visualization for the current top socket and staff height.

Validation:

- Collecting or triggering a test action adds a visible segment.
- Segments stack in a predictable order.
- Staff height and top socket update correctly.

Notes:

- Keep visual segments simple.
- Do not rely on final art assets or final mug shapes.

## Milestone 4: Physical Staff Collision

Goal: Let the growing staff physically interact with the level.

Planned work:

- Give each segment simple collision.
- Decide whether attached segments use kinematic collision, physics constraints, or a hybrid prototype.
- Add trace or overlap detection along the staff for early reliability if full physics is unstable.
- Expose collision profile, segment mass, angular damping, and stiffness tuning.
- Add simple wall/prop collision test objects to a prototype map.

Validation:

- Staff segments collide with walls and props.
- Tall staffs become harder to maneuver.
- Staff collision does not destabilize the wizard pawn immediately.

Notes:

- Start with the simplest reliable physical behavior.
- Full floppy simulation can come later after stress and snapping are testable.

## Milestone 5: Mana Slosh/Wobble System

Goal: Make taller staffs feel awkward without adding a separate mana resource.

Planned work:

- Add a wobble model driven by staff height, movement input, turning, and recent collisions.
- Apply wobble as visual staff sway first, then feed it into movement penalties if needed.
- Expose wobble strength, recovery rate, and height scaling to Blueprint.

Validation:

- Tall staffs visibly sway more than short staffs.
- Wobble affects control enough to be funny without making the pawn unusable.

Notes:

- Keep Mana Slosh abstract at first.
- Avoid complex fluid simulation.

## Milestone 6: Bonk Attack

Goal: Give players a simple staff-based interaction.

Planned work:

- Add a bonk input action.
- Sweep or swing the staff using a short animation, timeline, or collision trace.
- Detect hits against players, props, and environment.
- Add Blueprint-tunable bonk arc, cooldown, impulse, staff stress, and hit stun.
- Feed successful hits into the stress system once that exists.

Validation:

- Players can bonk props and other players locally.
- Bonk range scales with staff height or top reach.
- Cooldown and staff stress prevent constant spam.

Notes:

- Start with collision traces if physical swinging is unreliable.
- Do not build combat complexity beyond a party-game shove/bonk.

## Milestone 7: Staff Stress and Snapping

Goal: Make staff growth risky by tracking accumulated stress and breaking off the top.

Planned work:

- Add stress tracking to the staff system.
- Increase stress from wall impacts, bonks, sharp movement, wobble extremes, and being caught on props.
- Decay stress over time.
- Expose stress threshold, decay rate, impact scaling, and snap count to Blueprint.
- When stress exceeds the threshold, detach the top segment or top group.
- Spawn snapped segments as physics actors with simple impulse.
- Update the staff's current top socket after snapping.

Validation:

- Repeated collisions increase visible/debug stress.
- Exceeding the threshold snaps the top segment off.
- Detached segments simulate physics and no longer count toward staff height.

Notes:

- Snap one segment first.
- Multi-segment breaks can be added after the single-segment version is reliable.

## Milestone 8: Mug Collection Game Mode

Goal: Create the first playable objective loop.

Planned work:

- Add mug pickup actors.
- Mugs become staff segments.
- Spawn mugs around the test arena.
- Add respawn timing and simple pickup rules.
- Track each player's staff height, mug count, and snapped mug loss.

Validation:

- Players can collect mugs, gain staff segments, and build Mana Slosh.
- Mugs respawn or remain available enough to support a match.
- The game loop creates a clear tradeoff between growth, reach, and awkwardness.

Notes:

- Keep pickup meshes/materials obvious and placeholder.
- Avoid item rarity or power-up variety for the first playable.

## Milestone 9: Basic UI and Win Condition

Goal: Make the prototype playable without debug knowledge.

Planned work:

- Add a simple HUD for each local player.
- Show mana, staff height or mug count, stress, and match timer.
- Add match start, countdown, end state, and winner selection.
- Win condition: tallest staff at time limit, with tie broken by current mana or fewer snapped mugs.
- Add basic reset/restart flow for PIE testing.

Validation:

- Players understand their resource state and score.
- A timed local match can start, finish, and declare a winner.
- Resetting the match does not require restarting the editor.

Notes:

- Use plain debug-style UI first.
- Avoid menu polish until the core loop is fun.

## Suggested Implementation Order

1. Create or confirm the Unreal project scaffold.
2. Add a tiny local multiplayer test map.
3. Build wizard movement before staff growth.
4. Add socket-based staff stacking without physics.
5. Add staff collision and stress debug tools.
6. Add snapping.
7. Add mugs and collection rules.
8. Add bonk interactions.
9. Add UI and win condition.
10. Tune wobble, stress, and movement together.

## First Code Milestone Definition of Done

The first code milestone should be considered complete when:

- The Unreal project opens successfully.
- Two local players can spawn in a simple arena.
- Each player can move independently.
- Each player has a visible placeholder wizard body and staff root.
- No online multiplayer systems have been introduced.
