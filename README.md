# BresenhC

A feature-rich software renderer written in C with full 3D graphics pipeline implementation.

![perspective_correct_texture_mapping](https://github.com/user-attachments/assets/a11592fe-11ff-47a3-ba57-152743b8d4e7)

## Overview!

BresenhC is a from-scratch software renderer that implements a complete 3D graphics pipeline without relying on hardware acceleration or existing graphics APIs like OpenGL or DirectX. The entire rendering process is performed on the CPU. I created this soley as an academic exercise in understanding the fundamentals of 3D graphics. 
I used many resources for this such as Pikuma's 3D graphics course, his github can be found here: https://github.com/gustavopezzi as well as https://www.scratchapixel.com/index.html and many other online resources. 

## Features

### Core Rendering Capabilities
- **Multiple Rendering Modes**:
  - Wireframe (edges only)
  - Wireframe with vertices
  - Flat-filled triangles
  - Filled triangles with wireframe overlay
  - Textured triangles
  - Textured triangles with wireframe overlay

- **Lighting & Shading**:
  - None (raw colors)
  - Flat shading (single lighting calculation per face)
  - Gouraud shading (lighting calculated per vertex, interpolated across face)
  - Phong shading (normal interpolated per pixel, lighting calculated per pixel)

- **Visual Effects**:
  - Backface culling (skip rendering triangles facing away from camera)
  - Z-buffering for proper depth handling

### Additional Graphics Features
- **Perspective-Correct Texture Mapping** for accurate texture rendering
- **Camera Control Systems**:
  - First-person camera with mouse controls
  - Realistic camera movement with keyboard (WASD) navigation

- **3D Model Support**:
  - OBJ file format loading
  - glTF file format loading

- **Transformation Pipeline**:
  - Model matrix (object position/rotation/scale)
  - View matrix (camera position/orientation)
  - Projection matrix (perspective)
  - Viewport transformation

- **Clipping** to handle geometry intersecting the view frustum
- **Dynamic memory management** with custom array implementation

- **Matrix operations** (translation, rotation, scaling)
- **Vector mathematics library**
- **Triangle rasterization** with flat-top/flat-bottom decomposition
- **Digital Differential Analyzer** (DDA) line drawing
- **Barycentric coordinates** for interpolation

![wireframes](https://github.com/user-attachments/assets/400fc81b-d2a9-4e2f-a0c3-65ecc9efb2bd)

## Project Structure
- **Core Mathematics**
  - `brh_vector`: Vector operations (2D, 3D, 4D)
  - `brh_matrix`: Matrix operations and transformations
  - `math_utils`: General mathematical utilities

- **Graphics Pipeline**
  - `brh_display`: Display and buffer management
  - `brh_triangle`: Triangle rasterization and rendering
  - `brh_clipping`: View frustum clipping
  - `brh_light`: Lighting and shading models

- **Asset Management**
  - `brh_mesh`: 3D model data structure
  - `brh_mesh_manager`: Model resource management
  - `brh_texture_manager`: Texture resource management
  - `model_loader`: OBJ and glTF file importers
  - `upng`: PNG file format decoder

- **Scene Management**
  - `brh_camera`: Camera control systems
  - `brh_renderable`: Object instances with transforms
 
- ** 3rd Part Libraries Used **
  - SDL3
  - upng
  - external dynamic array.h file by pikuma

## Screenshots

### Rendering Modes

#### Wireframe
![wireframes](https://github.com/user-attachments/assets/3f89fb30-6c93-4089-83c2-731b4a1dafed)

#### Flat Shading
![flat_shading](https://github.com/user-attachments/assets/46db6185-dc67-4b0a-99b2-4c922c2c4fca)

#### Gourad Shading
![gourad_shading](https://github.com/user-attachments/assets/39f4ceea-7bfe-4a5c-bd33-75847ff2002b)

#### Phong Shading
![phong_shading](https://github.com/user-attachments/assets/02717304-5c4c-4488-99b3-79a1d2cdb7a2)

#### Perspective Correct Texture Mapping
![perspective_correct_texture_mapping](https://github.com/user-attachments/assets/c791dd51-bcbd-4e4a-9ddd-918dcc303fae)

### Techniques

#### Backface Culling Off
![back_face_culling_off](https://github.com/user-attachments/assets/9617d463-ea87-4005-ab67-21282e43d8df)

#### Backface Culling On
![backface_culling_on](https://github.com/user-attachments/assets/cfcdb8ef-bc39-4324-9400-81ce4e78392f)

#### Clipping Left Plane

![clipping_left_plane](https://github.com/user-attachments/assets/eb603cce-5a39-46af-b2f9-f9852cb6e446)

#### Clipping Near Plane
![clipping_near_plane](https://github.com/user-attachments/assets/1a8ec155-25ee-4f1b-947f-938ec432cd1b)

## Building & Running

### Prerequisites
- C compiler (MSVC, GCC, or Clang)
- SDL3 library
- CMake (optional, for cross-platform builds)

### Windows with Visual Studio
1. Clone the repository:
   ```
   git clone https://github.com/BeyondBelief96/BresenhC.git
   cd BresenhC
   ```

2. Open the solution file (`BresenhC.sln`) in Visual Studio

3. Build the solution (F7 or Build → Build Solution)

4. Run the executable (F5 or Debug → Start Debugging)

### Using CMake (cross-platform)
1. Clone the repository:
   ```
   git clone https://github.com/BeyondBelief96/BresenhC.git
   cd BresenhC
   ```

2. Create and navigate to a build directory:
   ```
   mkdir build
   cd build
   ```

3. Generate build files:
   ```
   cmake ..
   ```

4. Build the project:
   ```
   cmake --build .
   ```

5. Run the executable:
   ```
   ./BresenhC  # Linux/macOS
   BresenhC.exe  # Windows
   ```

### Controls

- **Mouse Left Click**: Toggle mouse camera control
- **WASD**: Move camera forward/backward/left/right
- **Space/Left Ctrl**: Move camera up/down
- **Mouse Movement**: Look around (when mouse is locked)
- **Escape**: Exit application

- **1-6**: Switch between rendering modes
  - 1: Wireframe with vertices
  - 2: Wireframe
  - 3: Filled triangles
  - 4: Filled triangles with wireframe
  - 5: Textured triangles
  - 6: Textured triangles with wireframe

- **F1-F4**: Switch between shading methods
  - F1: No shading
  - F2: Flat shading
  - F3: Gouraud shading
  - F4: Phong shading

- **C**: Enable backface culling
- **X**: Disable backface culling

## Implementation Details

### Rendering Pipeline
1. **Object Space**: Model vertex data with local coordinates
2. **World Space**: Apply model transformations (position, rotation, scale)
3. **Camera Space**: Apply view transformation (camera position and orientation)
4. **Clipping Space**: Apply projection transformation and clip against view frustum
5. **NDC Space**: Perspective division (w-divide)
6. **Screen Space**: Apply viewport transformation
7. **Rasterization**: Convert primitives to pixels with correct attributes
8. **Fragment Processing**: Apply lighting, texturing, and z-testing
9. **Display**: Present rendered image

### Performance Optimizations
- Flat-top/flat-bottom triangle decomposition for efficient rasterization
- Z-buffer for early depth rejection
- Efficient memory management with custom array implementation
- Perspective attribute pre-calculation to minimize per-pixel operations

### Mathematical Foundation
The renderer implements all necessary mathematical operations for 3D graphics:
- Vector operations (addition, subtraction, dot product, cross product, normalization)
- Matrix operations (multiplication, transposition, inversion)
- Transformations (translation, rotation, scaling)
- Projection (perspective, orthographic)
- Interpolation (linear, barycentric)

## Credits

This project was created as an educational deep dive into computer graphics fundamentals, inspired by many excellent resources on 3D rendering.

## License

This project is licensed under the MIT License - see the LICENSE file for details.
