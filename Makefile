all:
	mkdir -p build
	cd build; cmake ..; make -j$(shell nproc);

run: all
	cd build; ./Plex-Scrobbler

fclean:
	rm -rf build

re: fclean all
