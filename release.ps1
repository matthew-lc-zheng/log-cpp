cmake -B build -A x64
cmake --build build --config Release
if(Test-Path release){
    rm release\* -Force -Recurse
}else{
    md release
}
pushd release
md include
md lib
popd
$lib_path=".\build\Release\log.lib"
$include_path=".\include"
if(Test-Path $lib_path){
cp $lib_path .\release\lib
}else{
    Write-Host "log.lib does not exist"
}

if(Test-Path $include_path){
cp $include_path .\release\ -Recurse -Force
}else{
    Write-Host "include folder does not exist"
}




