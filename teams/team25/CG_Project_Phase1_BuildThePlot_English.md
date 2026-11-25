# Computer Graphics — Project Phase 1: CityLab · Build the Plot

**Complete English translation of the 10-page project specification**

> **Source:** *CG_Project_Phase1_BuildThePlot.pdf*, Department of Computer Engineering, Amirkabir University of Technology (Tehran Polytechnic). This document translates and reorganizes the original Persian text into readable Markdown. Its requirements, examples, tables, scores, and technical instructions are retained. Figures are described in text so this is a self-contained `.md` file. Dates are retained in the Solar Hijri calendar used in the original.

## Project information (original page 1)

| Field | Value |
|---|---|
| Course | Computer Graphics |
| Project | Phase 1 — **CityLab: Build the Plot** |
| University | Amirkabir University of Technology (Tehran Polytechnic), Department of Computer Engineering |
| Semester | First semester of academic year **1405–1406** (Solar Hijri) |
| Publication date | **20 Aban 1405** (Solar Hijri) |
| Deadline | **12 Azar 1405** (Solar Hijri) |
| Instructor | **Dr. Fatemeh Panahi** |
| Phase grade | **100 points + up to 10 bonus points** |
| Submission | **GitHub and the Courses system** |
| Work format | **Group project: two-person team** |
| Programming language and tools | **C++17 and classic OpenGL** |
| Repository | <https://github.com/MajidNami/Computer-Graphics-Fall-2026> |

### Phase overview

In Phase 1, you model your team's plot in the **CityLab** city: the ground, paths, entrance, and structures that represent your selected theme. All components are **static**, and all objects are built using **geometry and geometric transformations only**. By the end of the phase, the plot must be a complete place that a visitor can enter through the gate and walk around in.

---

## 1. Objective of this phase (original page 1)

The objective is to apply geometric-transformation concepts to modeling a real-world scene. At the end of this phase, you should be able to:

1. Model each object in its **own local coordinate system** and place it in the plot through a **composite transformation**.
2. Predict the effect of **transformation order** before executing the program, and express every sequence of `glTranslatef`, `glRotatef`, and `glScalef` calls as a **matrix product**.
3. Rotate an object about a point **other than the origin** by composing transformations.
4. Draw the **same geometry repeatedly** with different transformations, without duplicating its vertices.
5. Build a complex structure with **nested coordinate systems** and correctly manage the matrix stack with `glPushMatrix` and `glPopMatrix`.
6. Model a shape **vertex by vertex** if it does not exist in the provided library.

---

## 2. Prerequisites and tools (original pages 1–2)

This phase builds on the following course topics:

- Two-dimensional and three-dimensional transformations and **homogeneous coordinates**.
- Composition of transformations and **inverse transformations**.
- Introduction to **OpenGL** and **SceneLab**.
- **Hierarchical scene** structure.

The city provides the **camera, sun, fog, and automatic testing**. Your responsibility is **only to draw your own plot**.

### Allowed versus prohibited tools

| Allowed | Not allowed |
|---|---|
| **C++17** and **classic OpenGL**, including `glBegin`, `glEnd`, `glVertex3f`, `glColor3f`, and the matrix stack. | **VBO**, **VAO**, **framebuffer**, and **shaders**. |
| The **city SDK** (`citylab/CityLab.h`), its predefined `shapes::` shapes, and **GLM** for calculations. | `glClear`, `glLoadIdentity`, or any modification to the **camera**, **projection**, **viewport**, or **fog**. |
| Ready-made `props::` objects (e.g., benches, lights, and flowerbeds), **only to complete/decorate the environment**. | Loading ready-made models such as **OBJ** and **FBX**, or using another graphics library. |
| The **C++ standard library**. | Modifying files **outside** `teams/teamNN/`. |

### Project setup and commands

The project repository is:

<https://github.com/MajidNami/Computer-Graphics-Fall-2026>

If you have not yet run the city on your computer, follow the repository's `README.md`. Then create your team's directory by running the `citylab.py new teamNN` command, and write all your source code in:

```text
teams/teamNN/src/
```

In this document, `teamNN` is your team's identifier; for example, `team07`.

You will run these three commands many times throughout the phase:

```bash
python3 tools/citylab.py run teamNN         # build, then start at your plot
python3 tools/citylab.py check teamNN       # the checks the graders run
python3 tools/citylab.py pack teamNN --phase 1  # the zip for Courses, in dist/
```

**Windows:** use `py tools\citylab.py` instead of `python3 tools/citylab.py`.

---

## 3. The plot and its coordinate system (original pages 2–3)

When the city calls your `draw()` function, the **modelview matrix is already configured** so that you draw in the plot's **local coordinate system**, in **meters**. The origin is at the **center of the plot at ground level**, and the gate is always on the **`+z` side**, facing the street. This means your code must work unchanged regardless of the plot's position or orientation in the city (**Figure 1**).

### Plot bounds and orientation

- The **`x` axis** runs across the width of the plot from **`-40` to `+40` meters**. In a **double plot**, it runs from **`-80` to `+80` meters**.
- The **`z` axis** runs from the **back of the plot (`-50`)** to the **gate (`+50`)**.
- The **`y` axis** extends from ground level up to **40 meters**. You may also model down to **5 meters below the ground** (for example, a swimming pool or basement).
- When you stand in the street facing the plot, **`+x` is to your right** and **`-z` points forward into the plot**.
- Always read the plot dimensions from **`ctx.plot()`**, using `plot.width / 2` and `plot.depth / 2`. **Do not hard-code `40` and `50`** into your source. This ensures that the same code works if your team is assigned an adjacent plot and the plot becomes larger.
- The city **does not draw ground beneath your plot**. Creating the plot's ground is part of your work.
- Any component extending beyond the plot boundaries is clipped by the city's **clipping planes**, and `check` reports a warning.
- The gate is **10 meters wide** and centered on the **`+z` edge**. `check` verifies that visitors can enter the plot from the street; **do not block the gate**. The city itself draws the sign above the gate.
- City **lighting is already enabled**, and `glColor3f` determines each surface's color. **Lighting and materials are not evaluated** in this phase; selecting appropriate colors is enough.

### Figure 1 — Top view of the plot (original page 3)

The original figure shows the local coordinate frame that the city establishes before calling `draw()`:

```text
             x = -40         80 m total width          x = +40
                 +--------------------------------------+
                 |      BACK: z = -50                  |
                 |                                      |
                 |      y: 0 to +40 m above ground     |
                 |         0 to -5 m below ground      |
                 |                                      |
                 |       origin (0, 0, 0)              |  100 m
                 |       y+ points out of the page     |  depth
                 |                  +-----> +x         |
                 |                  |                   |
                 |                  v +z                |
                 |                                      |
                 +-----------------+      +-------------+
                                   GATE
                              width = 10 m
                              z = +50
             ================= STREET =================
```

For a **double plot**, the total width is **160 m**, with `x` from **-80 to +80**. The diagram emphasizes that the **gate is centered at `x = 0`**, on the street-facing edge.

---

## 4. Phase requirements (original pages 3–7)

By the end of the phase, the plot must satisfy **Sections 1–6** below. Each section tests one of the concepts taught in class, and its score is shown in its heading. **Section 7** evaluates code structure and compatibility with the city.

**Table 1** brings together the minimum requirements for a complete plot. These are the **threshold for completeness, not evidence of design quality**. Design quality is scored separately using the criteria in **Section 5: Idea and Creativity**.

### Table 1 — Minimum requirements for a complete Phase 1 plot (original pages 3–4)

| Item | Minimum required | Requirement section |
|---|---|---|
| **Ground and paths** | Design the **entire surface** of the plot. Leave **no large uniform/empty area exceeding 20 × 20 m**. Paths must connect the gate to **every main structure**. | 1 |
| **Main structures** | At least **3 theme-appropriate structures**, with at least **8 parts each**. At least one structure must have **2 or more floors**, or be **taller than 8 m**. | 1 |
| **Landmark object** | At least **1 object** built from **5 or more parts**, **at least 4 m tall**, visible from the street. | 1 |
| **Basic transformations** | Use **all three transformations** in several places; use **non-uniform scaling** on at least **3 parts**; include **at least one rotation around an axis other than `y`**. | 2 |
| **Rotation around a point** | At least **2 parts**, each rotated around a point **other than its own center**. | 3 |
| **Geometry reuse** | At least **3 geometries you created yourself**, with **at least 5 instances of each**. | 4 |
| **Custom-made shapes** | At least **2 custom-made shapes used in the plot**, one having **at least 10 faces**. | 5 |
| **Hierarchy** | At least **1 structure** with **3 nested coordinate-system levels** and **at least 12 parts at the last level**. | 6 |

**Definition of a “part”:** A part is any shape that is drawn separately—one call to `shapes::`, or one custom-made shape. **`props::` objects do not count** as parts in Table 1.

**Double plots:** Teams assigned a **double plot** must create **at least 4 main structures** (rather than 3).

### 4.1. Section 1 — Plot design and completeness (**15 points**, original page 4)

From the perspective of a visitor entering through the gate, the plot must look like a **complete and coherent place**, not a collection of scattered objects on a lawn.

Of the 15 points in this section:

- **10 points** are for satisfying the minimums in the **first three rows of Table 1**.
- **5 points** are for **design quality**, evaluated using the criteria in **Idea and Creativity** (Section 5).

Requirements and guidance:

1. Design the **entire plot surface**. Make paths, paved surfaces, and green spaces visually distinct. The **main entrance must align with the gate**, and paths must connect it to every main structure.
2. Choose main structures suitable for your theme. For example, an **airport** might have a **terminal**, **control tower**, and **hangar**.
3. The landmark should communicate the theme of the plot to people in the street—for example, a **statue**, **vehicle**, or **mast/tower**.
4. The main façades of structures must face the street (**`+z`**), and **all parts must stay within the plot boundaries**.
5. Choose **realistic dimensions in meters**: doors are approximately **2 m high**, and each building floor is about **3–4 m high**. The visitor's eye is **1.7 m above the ground**, so incorrect scale is easy to notice.

### 4.2. Section 2 — Translation, rotation, and scale (**10 points**, original page 4)

All three basic transformations must have a clear role in modeling the plot.

1. Use each of **`glTranslatef`**, **`glRotatef`**, and **`glScalef`** in **multiple places** in the plot.
2. Use **non-uniform scaling with three unequal scale factors** on at least **3 parts** (for example, walls, columns, or roofs).
3. Include **at least one rotation around an axis other than `y`**, such as a sloped surface, tilted panel, or diagonal beam.
4. Every **`glPushMatrix`** must have a matching **`glPopMatrix`** so one object's transformations do not unintentionally affect subsequent objects.

### 4.3. Section 3 — Rotation around a point (**5 points**, original page 5)

Rotate at least **2 parts** in your plot by composing the transformations:

```text
M = T(p) · R · T(-p)
```

Here, **`p` is a point other than the part's own center** (see Figure 2).

1. Appropriate examples include a **half-open door rotating around its hinge**, an **open parking barrier**, a **solar panel tilted about its bottom edge**, or an **airport passenger boarding bridge rotated around its support toward an aircraft**.
2. Express the pivot point **`p` in that part's local coordinate system**.
3. Mark the three corresponding calls in your source with the comments **`T(p)`**, **`R`**, and **`T(-p)`**.
4. The **rotation angle is fixed**. Choose parts for which pivoting around this point makes sense in the real world.

#### Figure 2 — Door leaf rotating around its hinge (original page 5)

The original figure shows a door leaf with a hinge (orange point) that remains at the same location after the rotation:

| Stage in the figure | Meaning |
|---|---|
| Door leaf in its local frame | The hinge is at **`p = (-0.6, 0, 0)`**. |
| **`T(-p)`** | Move the hinge to the origin. |
| **`R_y(70°)`** | Rotate the leaf **70° about the origin**. |
| **`T(p)`** | Move the hinge back; **the hinge stays fixed in the plot**. |

The figure gives the composite transformation:

```text
M = T(p) · Ry(70°) · T(-p)
```

The orange dot marks the hinge. The diagram is viewed from above.

### 4.4. Section 4 — Reusing geometry (**10 points**, original page 5)

Define a shape **once** and draw it **multiple times with different transformations**. This is the principle of **“one geometry, multiple transformations.”**

1. Create at least **3 geometries of your own**, each drawn **at least 5 times** with different transformations.
2. Each geometry must be a **function you wrote** that is made of **several shapes**—for example, a tree, light pole, chair, column, or window. **A single `shapes::box` is not sufficient.**
3. **Do not duplicate the vertices**. Instead, call the same function multiple times with a different transformation matrix each time.
4. Instances must not vary by translation alone. At least **some must have different rotations or scales**.
5. Choose instance positions through **data** (for example, an array containing each instance's specifications) or a **rule/loop** calculating positions in a row, grid, or circle.

### 4.5. Section 5 — Custom-made shapes (**10 points**, original pages 5–6)

The provided `shapes::` primitives are suitable for getting started, but not every structure can be constructed from boxes and cylinders. You must create at least **2 shapes that are not available in `shapes::`**, defining their geometry **vertex by vertex**. **At least one** of these must have **10 or more faces**.

1. Build the shapes with **`glBegin`** and **`glEnd`**, using one of the following modes:
   - `GL_TRIANGLES`
   - `GL_QUADS`
   - `GL_TRIANGLE_STRIP`
   - `GL_QUAD_STRIP`
   - `GL_TRIANGLE_FAN`
2. Examples: a **single-pitch roof**, **ramp**, **stairs**, **prism**, **arch**, **factory sawtooth roof**, **stepped stadium seating**, or **aircraft wing**.
3. Model each shape in its **own local coordinate system**, around an appropriate origin, so it can be placed using a simple transformation.
4. A shape must be **closed**, with **no visible face missing**. Give each face's vertices in an order that appears **counterclockwise when viewed from outside**. OpenGL uses this winding to distinguish a face's **front** from its **back**.
5. **Both custom shapes must actually appear in the plot**; merely defining their functions does not meet the requirement.

**Important lighting note:** The city's lighting is active, and each vertex uses the **most recently specified normal**. To make the colors of a custom-made shape consistent and predictable, call the following **once before `glBegin`**:

```cpp
glNormal3f(0.0f, 1.0f, 0.0f);
```

Calculating the mathematically correct normal for **each individual face is not required** in this phase.

### 4.6. Section 6 — Hierarchical structure (**10 points**, original page 6)

Construct at least **one main structure** using at least **3 levels of nested coordinate systems** and at least **12 parts at the final level**.

A possible example is a **building** where each floor is defined within the building's coordinate system, and windows are defined in each floor's coordinate system (**Figure 3**).

Rules and evaluation:

1. Each level must be defined **only relative to its parent level**.
2. Changing only the **whole structure's transformation**—translating or rotating it—must move **all its children together**, without modifying each child's placement.
3. During the **submission/demo session**, the teaching assistant will ask you to make this change in the code and show the result.
4. The building in Figure 3 is only an **illustration of the required structure**. Your hierarchy must fit **your chosen theme**: for example, an airplane at an airport or spectator stands at a stadium (see Table 2).

#### Figure 3 — Coordinate-system tree for a simple building (original page 6)

The original diagram contains this hierarchy:

```text
plot frame (set by the city before draw())
└── building
    ├── floor f
    │   ├── storey (walls and slab)
    │   └── window w
    └── entrance ramp
```

- **Building:** transformation `T_b` places the building in the plot.
- **Floor `f`:** transformation `T_f` moves into floor `f`.
- **Window `w`:** transformation `T_w` places a window within the floor.
- **Entrance ramp:** transformation `T_r` positions and sizes the ramp under the building.

A window's transformation is the product along the path from the root to that window:

```text
M_window = T_b · T_f · T_w
```

**Guidance on scaling:** Use **non-uniform scaling at the leaves** of the hierarchy, **not at parent nodes**. If a parent is stretched along one axis and a child is rotated, the child can become skewed or distorted because the parent's scaling acts on the child after the child's rotation.

### 4.7. Section 7 — Code structure and city compatibility (**30 points**, original pages 6–7)

This section assesses the quality of your code and the plot's compatibility with CityLab. Later phases will be built on this code, so good structure matters from the start.

#### Separate state/data from geometry

- Drawing functions should draw their shapes **only in their local coordinate systems**.
- Store each object's **position, rotation angle, and size as data**—for example, in a **`struct`**, array, or named constants—**not** by embedding the final world-space coordinates directly into individual vertex coordinates.

#### Readable and maintainable code

- Give each structure **its own drawing function**.
- Use **meaningful English names**.
- Define **named constants** for repeatedly used numbers.
- Keep comments **short and in English**.
- Put **all classes** in the **`teamNN` namespace**.

#### `draw()` only draws

- The `draw()` method is **`const`**.
- Any computation that only needs to happen **once** should be performed in **`init()`**, not in `draw()`.

#### City contracts and automated checks

- The result of **`check` must be `PASS`**, with **no `FAIL` lines**.
- Fix warnings as far as possible.
- **`lint` must report no errors**.
- **All vertices must be inside the plot boundaries**.
- The **gate must remain open and traversable**.
- Each rendering must use **no more than 100,000 vertices**.
- **Each `check` error reduces** this section's score.
- Code that **does not compile** or **stops/crashes during execution** receives **no points for this section**.

**Vertex-budget guidance:** The 100,000-vertex limit is sufficient for a plot, but use it carefully. A `shapes::sphere` with its default tessellation has about **2,300 vertices**, whereas `shapes::sphere(0.5f, 12, 8)` has about **600 vertices**. Use lower tessellation for small objects that appear many times. The **`vertex budget`** line in the `check` output reports the exact vertex count.

### Optional bonus tasks — up to **10 additional points** (original page 7)

These items were not covered in the slides or go one step beyond the taught material. For **every bonus task you implement**, explain it in your **report** and identify where it appears in the **code**. **No explanation means no points for that item.**

**Bonus points are awarded only if the plot satisfies all minimum requirements in Table 1.**

#### Bonus 1 — Display list for static components (**3 points**)

- Record **all fixed geometry in the plot** **once in `init()`**, in a **display list**.
- In `draw()`, **only call that display list**.
- Compare the execution time of `draw()` **before and after** the change, using timings shown by **`check`** and the **F3 key**. Include the comparison in your report.
- A display list **does not reduce the vertex count**; it only makes vertex submission faster.
- **Nested `glNewList` calls are not allowed**.

#### Bonus 2 — Custom matrix with `glMultMatrixf` (**2 points**)

- Implement a transformation for which OpenGL **does not provide a ready-made function**; for example, **shearing** to make a slanted façade or a tower leaning toward one side.
- Write the transformation as a **4 × 4 matrix** and multiply it into the current matrix using **`glMultMatrixf`**.
- Include the matrix in the report and identify **where each entry is placed** in the **16-element array** passed to OpenGL.

#### Bonus 3 — Recursive tree (**3 points**)

- Create a **tree or plant** drawn by a **recursive function**.
- Each branch must draw **two or more child branches** in its own coordinate system (the **L-system idea**).
- Report the **recursion depth** and **vertex count**, and respect the overall vertex budget.

#### Bonus 4 — Surface of revolution (**2 points**)

- Create a shape generated by rotating a **two-dimensional profile around the `y` axis**.
- Compute the surface vertices **yourself using the `R_y` rotation matrix**.
- Suitable examples include a **dome**, **water tower**, **decorative/turned column**, or **tank**.

---

## 5. Idea and creativity (original pages 7–8)

The minimums in Table 1 guarantee the **completeness** of the plot, but **not its quality**. The **5 design-quality points in Section 1** are awarded based on the **entire plot**, not an individual object. Merely adding an unusual object or changing colors does not earn these points.

Design-quality criteria:

1. **A clear concept.** A visitor should understand what type of place the plot represents **without reading a sign**, and all components should support that concept.
2. **Functional layout.** Place components as they would work in a real location. For example, an airport should have clear connections among the **terminal, aircraft parking area, and runway**.
3. **Scale and proportions.** Components should be proportional to one another and to human size; objects should not float in the air or unintentionally intersect one another.
4. **Human-scale details.** Add details along paths and near entrances that are visible from normal eye height, such as **benches, lights, fences, signs, and canopies**.
5. **Entrance view and compatibility with the city.** The first view from the gate is the most important. A strong central axis, distinctive entrance, or distant visible landmark establishes identity. Plot colors and scale should be consistent with the city's **modern, bright environment**.

**What creativity means here:** Use course concepts to better express the plot's theme. For instance, arrange rows of stadium seats with a loop and a simple relationship so that changing one number alters the whole seating layout, or build a distinctive roof using a custom-made shape.

**Table 2** suggests how the phase's concepts might be used for several themes from the catalog. These are **suggestions only**; your own ideas are more valuable.

### Table 2 — Examples of concepts applied to different themes (original page 8)

| Theme | Hierarchical structure | Custom-made shape | Geometry reuse | Rotation around a point |
|---|---|---|---|---|
| **Airport** | Airplane: fuselage, wings, engines | Arched hangar roof | Lights along the runway | Steps for boarding an airplane |
| **Wind farm** | Turbine: tower, nacelle, hub, blades | Tapered turbine blade | Turbines and blades | Open hatch of the nacelle |
| **Football stadium** | Stands: rows, seats | Stepped platforms/stands | Seats and lighting masts | Canopy above the stands |
| **Public library** | Bookshelf: shelves and books | Sawtooth roof | Reading-room tables and chairs | Partially open roof window/skylight |
| **Weather station** | Anemometer: support, arms, cups | Radar dome | Measurement/sensor enclosures | Tilted antenna |
| **Fueling and charging station** | Canopy: pillars, roof, lights | Sloped canopy roof | Fuel pumps and chargers | Entrance barrier |

If your theme is **not in the table**, complete these **same four concept columns** for your own theme. This is a good starting point for designing your plot.

---

## 6. Common mistakes (original page 9)

Most difficulties in this phase are included in **Table 3**. Check this table **before asking a question**.

### Table 3 — Common Phase 1 problems and how to resolve them

| Problem | Cause | Solution |
|---|---|---|
| **Half of a box is below the ground.** | `box` and `sphere` are drawn around their **centers**. | Move the object up by **half its height**. `cylinder` and `cone` are placed on the ground. |
| **Angles are incorrect.** | `glRotatef` uses **degrees**, while `std::sin` uses **radians**. | Convert degrees to radians by multiplying by **`π / 180`**. |
| **Two overlapping surfaces flicker.** | Both surfaces are at the **same height** (**z-fighting**). | Leave a gap of a few centimeters; for example, put grass at **`0.01`** and a path at **`0.02`**. |
| **An object's transformation also affects later objects.** | A `glPushMatrix` is missing its corresponding `glPopMatrix`. | Count the push/pop pairs. The **`OpenGL usage`** line in `check` also reports this error. |
| **`lint` fails.** | Using `glLoadIdentity`, `glClear`, or changing the camera. | Use **only `glPushMatrix` and `glPopMatrix`** for the matrix management described here; do not make the prohibited calls or changes. |
| **Structures are displaced when using a double plot.** | Constants such as **`40` or `50`** were written directly into the code. | Read dimensions from **`plot()`** instead. |
| **A custom-made shape is dark or unevenly colored.** | No normal was specified for it. | Call **`glNormal3f(0, 1, 0)` before `glBegin`**. |

---

## 7. Report requirements (original pages 9–10)

Write the Phase 1 report in your team's **`README.md`**. The `citylab.py new` command has already created a template for it.

- Place figures/screenshots in **PNG or JPG** format inside:

  ```text
  teams/teamNN/report/
  ```

- The **entire team directory must be no larger than 15 MB**.
- Focus your report on **your design decisions** and on the **connection between mathematics and code**.
- **Do not repeat the lecture slides**.

Your report must include **all** of the following:

1. **Team and concept information:** names and student ID numbers of **both members**, the **theme**, the **plot number**, and a **one-paragraph description** of the plot's concept.
2. **Top-down layout:** a simple overhead map of the plot—either the **aerial image generated by `check`** or a hand-drawn plan—with the **structures labeled**.
3. **Table of minimum requirements:** for **each row of Table 1**, give the actual value achieved in your plot (for example, number of structures or instances) and the **source file and function** implementing it.
4. **Math in Code:** write one transformation sequence **from your own plot** as a product of matrices. Explain the purpose of **each factor** and identify the relevant location in your code. A suitable example, including a rotation about a point, is:

   ```text
   M = T(p0) · Ry(30°) · T(p) · Rx(-20°) · T(-p) · S(4, 0.5, 2)
   ```

5. **Manual calculation of one vertex:** select a vertex from **one of your custom-made shapes**, multiply it by **the same matrix** used above, and calculate its final coordinates in the plot coordinate system. Show the calculation **step by step**. To verify the result, draw a **small sphere at the computed point** and provide a screenshot showing it **overlaps the vertex**.
6. **Transformation-order experiment:** swap the order of **two transformations in one sequence**. Include **before-and-after screenshots** and explain **why the result changes**.
7. **Coordinate-system hierarchy tree:** include a diagram of the **Section 6** structure's coordinate systems, with the **transformation at every level**, similar to **Figure 3**.
8. **At least three screenshots** captured using **F12**:
   - One **from the gate**, at the visitor's **eye height**.
   - One **from above**.
   - One **close-up of the landmark object**.
9. **Contributions and sources:** describe each member's **share of the work**, list **external sources**, and identify any **AI tools used**, together with **how each tool was used**.

---

## 8. Scoring and evaluation (original page 10)

The grade for Phase 1 is calculated as follows:

- **60%:** plot design and implementation of the course concepts (**Sections 1–6**).
- **30%:** code structure and compatibility with the city (**Section 7**).
- **10%:** the **report**.

If a section's minimums are not met, the score for that section is **reduced in proportion to the work completed**.

### Table 4 — Phase 1 grading rubric

| Section | Evaluation criterion | Points |
|---|---|---:|
| **1. Plot design and completeness** | First 3 rows of Table 1 (**10 points**) and design quality (**5 points**). | **15** |
| **2. Translation, rotation, and scale** | All 3 transformations; non-uniform scaling in **3 parts**; rotation around an axis other than `y`. | **10** |
| **3. Rotation around a point** | `T(p) · R · T(-p)` in **2 parts**, clearly identified in code. | **5** |
| **4. Geometry reuse** | **3 self-made geometries** with at least **5 instances each**; positions chosen by data or a rule. | **10** |
| **5. Custom-made shapes** | **2 closed, vertex-defined shapes**, one with **10 or more faces**, both used in the plot. | **10** |
| **6. Hierarchical structure** | **3 nested levels** and **12 parts at the last level**; parent-transformation/movement test. | **10** |
| **7. Code structure and city compatibility** | State/geometry separation, readability, and error-free `check` and `lint`. | **30** |
| **Report** | Minimums table, mathematics in code, vertex calculation, transformation-order test, hierarchy tree, and screenshots. | **10** |
| **TOTAL** | | **100** |

**Bonus:** Up to **10 additional points** from the optional bonus tasks can be added on top of this **100-point** total.

### Submission/demo session

There will be a **project submission and demonstration session**.

- During the session, the **teaching assistant** will run your plot in the city, review the **`check` report**, and ask **both members** questions about the code and transformations used in the plot.
- **Attendance by both team members is mandatory**.
- Each member must be able to **explain every part of the team's code**, **not only the part they personally wrote**.
- The two members normally receive the **same grade**. However, if the **commit history** or a member's answers during the session suggest that their contribution was small or that they do not understand the team's code, that member's **grade may be determined separately**.

**Good luck — Computer Graphics course teaching team.**

---

*End of the English translation of the original 10-page Phase 1 project brief.*
