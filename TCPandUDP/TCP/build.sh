rm -rf ./bin/

curr_work_path=`pwd`
build_dir_path="$curr_work_path/build/"
echo ${build_dir_path}

rm -r -f ${build_dir_path}
mkdir ${build_dir_path}

cd ${build_dir_path}
curr_work_path=`pwd`
echo ${curr_work_path}

cmake ..

make

