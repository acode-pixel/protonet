# How to build
**Clone repo**  
`git clone https://github.com/acode-pixel/protonet`  
**Setup conan**  
```
pip install conan
conan profile detect
conan install .
```
**Build**
```
cmake --preset=conan-<build_preset>
cmake --build ./build/<preset_buildDir>
```
