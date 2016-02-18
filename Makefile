all: WindowTest GraphicsTest WaterTest

Testbed.o: include/Testbed.hpp include/Input.hpp src/Testbed.cpp
	g++ -g -std=c++11 -Wall -c src/Testbed.cpp -Iinclude
	
Graphics.o: include/Graphics.hpp src/Graphics.cpp
	g++ -g -std=c++11 -Wall -c src/Graphics.cpp -Iinclude

Shader.o: include/Graphics.hpp include/Shader.hpp src/Shader.cpp
	g++ -g -std=c++11 -Wall -c src/Shader.cpp -Iinclude

Camera.o: include/Camera.hpp src/Camera.cpp
	g++ -g -std=c++11 -Wall -c src/Camera.cpp -Iinclude

Mesh.o: include/Mesh.hpp src/Mesh.cpp
	g++ -g -std=c++11 -Wall -c src/Mesh.cpp -Iinclude

Texture.o: include/Texture.hpp src/Texture.cpp lodepng.o
	g++ -g -std=c++11 -Wall -c src/Texture.cpp -Iinclude

Models.o: include/Models.hpp src/Models.cpp
	g++ -g -std=c++11 -Wall -c src/Models.cpp -Iinclude

lodepng.o: include/lodepng.h src/lodepng.cpp
	g++ -g -std=c++11 -Wall -c src/lodepng.cpp -Iinclude

LEAN.o: include/LEAN.hpp src/LEAN.cpp
	g++ -g -std=c++11 -Wall -c src/LEAN.cpp -Iinclude

RenderTarget.o: include/RenderTarget.hpp src/RenderTarget.cpp
	g++ -g -std=c++11 -Wall -c src/RenderTarget.cpp -Iinclude

Generator: LEAN.o lodepng.o src/Generator.cpp
	g++ -g -std=c++11 -Wall -o res/Generator src/Generator.cpp LEAN.o lodepng.o -Iinclude

WindowTest: Testbed.o Graphics.o tests/WindowTest.cpp
	g++ -g -std=c++11 -Wall -o WindowTest tests/WindowTest.cpp Testbed.o Graphics.o -Iinclude -lglfw -lGLEW -lGL

GraphicsTest: Testbed.o Graphics.o Shader.o Camera.o tests/GraphicsTest.cpp
	g++ -g -std=c++11 -Wall -o GraphicsTest tests/GraphicsTest.cpp Testbed.o Graphics.o Shader.o Camera.o -Iinclude -lglfw -lGLEW -lGL

WaterTest: Testbed.o Graphics.o Shader.o Camera.o Mesh.o Texture.o tests/WaterTest.cpp
	g++ -g -std=c++11 -Wall -o WaterTest tests/WaterTest.cpp Testbed.o Graphics.o Shader.o Camera.o Mesh.o Texture.o lodepng.o -Iinclude -lglfw -lGLEW -lGL

TextureTest: Testbed.o Graphics.o Shader.o Camera.o Mesh.o Texture.o tests/TextureTest.cpp
	g++ -g -std=c++11 -Wall -o TextureTest tests/TextureTest.cpp Testbed.o Graphics.o Shader.o Camera.o Mesh.o Texture.o lodepng.o -Iinclude -lglfw -lGLEW -lGL

ModelTest: Testbed.o Graphics.o Shader.o Camera.o Models.o Texture.o tests/ModelTest.cpp
	g++ -g -std=c++11 -Wall -o ModelTest tests/ModelTest.cpp Testbed.o Graphics.o Shader.o Camera.o Models.o Texture.o lodepng.o -Iinclude -lglfw -lGLEW -lGL

LEANTest: Testbed.o Graphics.o Shader.o Camera.o Mesh.o Texture.o LEAN.o RenderTarget.o tests/LEANTest.cpp
	g++ -g -std=c++11 -Wall -o LEANTest tests/LEANTest.cpp Testbed.o Graphics.o Shader.o Camera.o Mesh.o Texture.o lodepng.o LEAN.o RenderTarget.o -Iinclude -lglfw -lGLEW -lGL
