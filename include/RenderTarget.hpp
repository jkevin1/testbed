#ifndef RenderTarget_HPP
#define RenderTarget_HPP

#include <Graphics.hpp>
#include <vector>

class RenderTarget
{
public:
    static RenderTarget create(GLuint width, GLuint height, int num=1, bool hasDepth=true);
    RenderTarget() : handle(0), depth(0) { }
    RenderTarget(RenderTarget&& rt);
    RenderTarget& operator=(RenderTarget&& rt);
    ~RenderTarget() { release(); }
    
    void clear();
    void blit(const RenderTarget& src, int width, int height);
    void activate() const;
    void release();

    size_t getNumTextures() { return textures.size(); }
    GLuint getTexture(int index) { return textures[index]; }
    GLuint getDepthTexture() { return depth; };
private:
    GLuint handle;
    std::vector<GLuint> textures; // Color attachments
    GLuint depth;  // Renderbuffer attachment
};

#endif
