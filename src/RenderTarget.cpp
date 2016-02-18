#include "RenderTarget.hpp"
#include "Testbed.hpp"

RenderTarget RenderTarget::create(GLuint width, GLuint height, int num, bool hasDepth)
{
    RenderTarget target;

    glGenFramebuffers(1, &target.handle);
    glBindFramebuffer(GL_FRAMEBUFFER, target.handle);
    printf("Creating framebuffer %u\n", target.handle);
    
    // Add attachments
    if (num > 0)
    {
        target.textures.resize(num);
        glGenTextures(num, &target.textures[0]);
        for (int i = 0; i < num; i++)
        {
            glBindTexture(GL_TEXTURE_2D, target.textures[i]);
        //    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
            glTexStorage2D(GL_TEXTURE_2D, 10, GL_RGB8, width, height);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0+i, GL_TEXTURE_2D, target.textures[i], 0);
            printf("\tColor Attachment %d: %u\n", i, target.textures[i]);
        }
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    if (hasDepth)
    {
        glGenTextures(1, &target.depth);
        glBindTexture(GL_TEXTURE_2D, target.depth);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT, target.depth, 0);
        printf("\tDepth Attachment: %u\n", target.depth);
    }

    Testbed::assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "Failed to create FBO");

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    return std::move(target);
}

RenderTarget::RenderTarget(RenderTarget&& rt)
{
    handle = rt.handle;
    rt.handle = 0;
    textures = std::move(rt.textures);
    depth = rt.depth;
    rt.depth = 0;
}

RenderTarget& RenderTarget::operator=(RenderTarget&& rt)
{
    release();
    handle = rt.handle;
    rt.handle = 0;
    textures = std::move(rt.textures);
    depth = rt.depth;
    rt.depth = 0;
    return *this;
}

void RenderTarget::clear()
{
    activate();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void RenderTarget::blit(const RenderTarget& src, int width, int height)
{
    glBindFramebuffer(GL_READ_FRAMEBUFFER, src.handle);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, handle);
    glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, 
                    GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT, GL_NEAREST);
}

void RenderTarget::activate() const
{
    static GLuint active = 0;
    if (active != handle)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, handle);
        active = handle;
    }
}

void RenderTarget::release()
{
    if (textures.size() > 0)
        glDeleteTextures(textures.size(), &textures[0]);
    textures.clear();
    if (depth)
        glDeleteRenderbuffers(1, &depth);
    depth = 0;
    if (handle)
        glDeleteFramebuffers(1, &handle);
    handle = 0;
}
