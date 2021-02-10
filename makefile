all:
	g++ a.cpp Shader.cpp Decoder.cpp -lavformat -lavcodec -lglfw -framework opengl -Wno-deprecated-declarations

clean:
	rm a.out
