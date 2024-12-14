# How to build
**Clone repo**  
`git clone https://github.com/acode-pixel/protonet`  
**Setup conan**  
```
pip install conan
conan profile detect
conan install . -s build_type=<Debug/Release>
```
**Build (Debug/Release) **
```
cmake --preset=conan-<build_preset>
cmake --build ./build/<preset_buildDir>
```
