#ifndef UniformBlock_HPP
#define UniformBlock_HPP

#include "Graphics.hpp"
#include "Shader.hpp"
#include <stdio.h>

template <typename T>
class UniformBlock {
 public:
  static UniformBlock create(const T* value=nullptr);
  UniformBlock() : binding(0), buffer(0) { }
  UniformBlock(UniformBlock&& ub);
  UniformBlock& operator=(UniformBlock&& ub);
  ~UniformBlock() { release(); }
  
  void release();
  T* map();
  void unmap();
  GLuint getBinding() { return binding; }
  
 private:
  GLuint binding;
  GLuint buffer;
};

template <typename T>
UniformBlock<T> UniformBlock<T>::create(const T* value) {
  static GLuint counter = 0;
  UniformBlock<T> block;
  block.binding = ++counter;
  glGenBuffers(1, &block.buffer);
  glBindBuffer(GL_UNIFORM_BUFFER, block.buffer);
  glBufferStorage(GL_UNIFORM_BUFFER, sizeof(T), value, GL_MAP_WRITE_BIT);
  glBindBufferRange(GL_UNIFORM_BUFFER, block.binding, block.buffer, 0, sizeof(T));
  printf("Created uniform block %u at binding %u\n", block.buffer, block.binding);
  return block;
}

template <typename T>
UniformBlock<T>::UniformBlock(UniformBlock&& ub) {
  binding = ub.binding;
  buffer = ub.buffer;
  ub.binding = 0;
  ub.buffer = 0;
}

template <typename T>
UniformBlock<T>& UniformBlock<T>::operator=(UniformBlock&& ub) {
  binding = ub.binding;
  buffer = ub.buffer;
  ub.binding = 0;
  ub.buffer = 0;
  return *this;
}

template <typename T>
void UniformBlock<T>::release() {
  if (buffer)
    glDeleteBuffers(1, &buffer);
  buffer = 0;
  binding = 0;
}

template <typename T>
T* UniformBlock<T>::map() {
  return (T*)glMapNamedBufferRange(buffer, 0, sizeof(T), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
}

template <typename T>
void UniformBlock<T>::unmap() {
  glUnmapNamedBuffer(buffer);
}

#endif
