fedora:
	sudo dnf install -y ninja-build muParser-devel
	git submodule set-url plugins https://github.com/ttys3/albertlauncher-plugins.git
	git submodule sync --recursive
	git submodule update --init --recursive --remote
	rm -rf ./build
	mkdir build && cd ./build && cmake ../ && make -j14
	sudo cp -v ./build/lib/libwebsearch.so /usr/lib64/albert/plugins/libwebsearch.so
