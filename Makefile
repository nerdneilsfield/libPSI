build_debug: create_dir
	cmake -B out/build/linux -S . -DFETCH_AUTO=ON -DENABLE_KKRT_PSI=ON -DENABLE_DRRN_PSI=ON -DCMAKE_BUILD_TYPE=Debug
	cmake --build out/build/linux -j$(nproc)

build_release: create_dir
	cmake -B out/build/linux -S . -DFETCH_AUTO=ON -DENABLE_KKRT_PSI=ON -DENABLE_DRRN_PSI=ON -DCMAKE_BUILD_TYPE=Release
	cmake --build out/build/linux -j$(nproc)

test_all:
	./out/build/linux/frontend/frontend.exe -u

create_dir:
	echo "-------crate dir--------"
	mkdir -p out/build/linux
	mkdir -p out/install/linux

clean:
	echo "-------clean--------"
	rm -rf out/*