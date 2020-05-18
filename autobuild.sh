#!/bin/bash
echo "Checking if prerequites are installed"
command -v cmake >/dev/null 2>&1 || { echo >&2 "I require cmake but it's not installed.  Aborting."; exit 1; }
B_SYSTEM="ninja"
EX_CM_ARGS="-GNinja"
command -v ninja >/dev/null 2>&1 || { echo >&2 "Ninja build command not found. Fallback to make";
B_SYSTEM="make";
EX_CM_ARGS='-GUnix Makefiles';
}
command -v make >/dev/null 2>&1 || { echo >&2 "Make build command not found. Aborting."; exit 1; }
INSTALLED_LIBS=0
echo "Checking libraries"

for LIB in $(cat dependencies.txt); do
    if [[ $(ldconfig -p | grep ${LIB}) != "" ]] >/dev/null 2>&1; then
        echo "$LIB is installed"
    else
        echo "Missing library ${LIB//[$'\t\r\n ']}"
        INSTALLED_LIBS=1
    fi

done
if [ $INSTALLED_LIBS -ne 0 ]; then
    echo "Some libraries not installed, try running ldconfig if you are sure they are installed"
    exit -1;
fi
echo "Entering build directory"
rm -rf build
[ $? -eq 0 ]  || { echo >&2 "Cannot clean build dir"; exit $?; }
mkdir build >/dev/null 2>&1
[ $? -eq 0 ]  || { echo >&2 "Cannot create build dir"; exit $?; }
cd build
[ $? -eq 0 ]  || { echo >&2 "Cannot open build dir"; exit $?; }
cmake "$EX_CM_ARGS" ..
[ $? -eq 0 ]  || { echo >&2 "Cmake failed"; exit $?; }
$B_SYSTEM
[ $? -eq 0 ]  || { echo >&2 "Build failed"; exit $?; }
echo "Successfuly built! Creating target folder"
cd ..
[ $? -eq 0 ]  || { echo >&2 "Cannot step back from build dir"; exit $?; }
rm -rf target
[ $? -eq 0 ]  || { echo >&2 "Cannot clean target dir"; exit $?; }
mkdir target >/dev/null 2>&1
[ $? -eq 0 ]  || { echo >&2 "Cannot create target dir"; exit $?; }
cd target
[ $? -eq 0 ]  || { echo >&2 "Cannot open target dir"; exit $?; }
exeFiles=()
for f in "../build"/*; do [[ -x $f && -f $f ]] && exeFiles+=( "$f" ); done
cp "${exeFiles[@]}" "."
[ $? -eq 0 ]  || { echo >&2 "Cannot copy files to target dir"; exit $?; }
echo "Done!"
cd ..
[ $? -eq 0 ]  || { echo >&2 "Cannot step back from target dir"; exit $?; }
while true; do
    read -p "Do you wish to clean build dir [Y/n]?" yn
    case $yn in
        [Yy]* )  rm -rf build; [ $? -eq 0 ]  || { echo >&2 "Couldn't clean build dir"; exit $?; }; echo "Done"; exit 0;;
        [Nn]* ) exit 0;;
        * ) echo "Please answer Y or n.";;
    esac
done
