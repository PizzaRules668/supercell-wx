#include <scwx/qt/map/overlay_layer.hpp>
#include <scwx/qt/gl/shader_program.hpp>
#include <scwx/qt/gl/text_shader.hpp>
#include <scwx/qt/util/font.hpp>

#include <chrono>
#include <execution>

#include <boost/date_time.hpp>
#include <boost/date_time/local_time/local_time.hpp>
#include <boost/log/trivial.hpp>
#include <boost/timer/timer.hpp>
#include <GeographicLib/Geodesic.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <mbgl/util/constants.hpp>

namespace scwx
{
namespace qt
{
namespace map
{

static const std::string logPrefix_ = "[scwx::qt::map::overlay_layer] ";

class OverlayLayerImpl
{
public:
   explicit OverlayLayerImpl(
      std::shared_ptr<view::RadarProductView> radarProductView,
      gl::OpenGLFunctions&                    gl) :
       radarProductView_(radarProductView),
       gl_(gl),
       textShader_(gl),
       font_(util::Font::Create(":/res/fonts/din1451alt.ttf")),
       shaderProgram_(gl),
       uMVPMatrixLocation_(GL_INVALID_INDEX),
       uColorLocation_(GL_INVALID_INDEX),
       vbo_ {GL_INVALID_INDEX},
       vao_ {GL_INVALID_INDEX},
       texture_ {GL_INVALID_INDEX},
       numVertices_ {0},
       colorTableUpdated_(false),
       plotUpdated_(true)
   {
      // TODO: Manage font at the global level, texture at the view level
   }
   ~OverlayLayerImpl() = default;

   std::shared_ptr<view::RadarProductView> radarProductView_;
   gl::OpenGLFunctions&                    gl_;

   gl::TextShader              textShader_;
   std::shared_ptr<util::Font> font_;
   gl::ShaderProgram           shaderProgram_;
   GLint                       uMVPMatrixLocation_;
   GLint                       uColorLocation_;
   GLuint                      vbo_;
   GLuint                      vao_;
   GLuint                      texture_;

   GLsizeiptr numVertices_;

   bool colorTableUpdated_;
   bool plotUpdated_;
};

OverlayLayer::OverlayLayer(
   std::shared_ptr<view::RadarProductView> radarProductView,
   gl::OpenGLFunctions&                    gl) :
    p(std::make_unique<OverlayLayerImpl>(radarProductView, gl))
{
}
OverlayLayer::~OverlayLayer() = default;

void OverlayLayer::initialize()
{
   BOOST_LOG_TRIVIAL(debug) << logPrefix_ << "initialize()";

   gl::OpenGLFunctions& gl = p->gl_;

   p->textShader_.Initialize();

   // Load and configure overlay shader
   p->shaderProgram_.Load(":/gl/overlay.vert", ":/gl/overlay.frag");

   p->uMVPMatrixLocation_ =
      gl.glGetUniformLocation(p->shaderProgram_.id(), "uMVPMatrix");
   if (p->uMVPMatrixLocation_ == -1)
   {
      BOOST_LOG_TRIVIAL(warning) << logPrefix_ << "Could not find uMVPMatrix";
   }

   p->uColorLocation_ =
      gl.glGetUniformLocation(p->shaderProgram_.id(), "uColor");
   if (p->uColorLocation_ == -1)
   {
      BOOST_LOG_TRIVIAL(warning) << logPrefix_ << "Could not find uColor";
   }

   if (p->texture_ == GL_INVALID_INDEX)
   {
      p->texture_ = p->font_->GenerateTexture(gl);
   }

   p->shaderProgram_.Use();

   // Generate a vertex array object
   gl.glGenVertexArrays(1, &p->vao_);

   // Generate vertex buffer objects
   gl.glGenBuffers(1, &p->vbo_);

   gl.glBindVertexArray(p->vao_);

   // Bottom panel (dynamic sized)
   gl.glBindBuffer(GL_ARRAY_BUFFER, p->vbo_);
   gl.glBufferData(
      GL_ARRAY_BUFFER, sizeof(float) * 6 * 2, nullptr, GL_DYNAMIC_DRAW);

   gl.glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, static_cast<void*>(0));
   gl.glEnableVertexAttribArray(0);

   // Bottom panel color
   gl.glUniform4f(p->uColorLocation_, 0.0f, 0.0f, 0.0f, 0.75f);

   connect(p->radarProductView_.get(),
           &view::RadarProductView::PlotUpdated,
           this,
           &OverlayLayer::ReceivePlotUpdate);
}

void OverlayLayer::render(const QMapbox::CustomLayerRenderParameters& params)
{
   gl::OpenGLFunctions& gl = p->gl_;

   static std::string plotTimeString;

   if (p->plotUpdated_)
   {
      using namespace std::chrono;
      auto plotTime =
         time_point_cast<seconds>(p->radarProductView_->PlotTime());

      if (plotTime.time_since_epoch().count() != 0)
      {
         zoned_time         zt = {current_zone(), plotTime};
         std::ostringstream os;
         os << zt;
         plotTimeString = os.str();
      }

      p->plotUpdated_ = false;
   }

   glm::mat4 projection = glm::ortho(0.0f,
                                     static_cast<float>(params.width),
                                     0.0f,
                                     static_cast<float>(params.height));

   p->shaderProgram_.Use();

   gl.glUniformMatrix4fv(
      p->uMVPMatrixLocation_, 1, GL_FALSE, glm::value_ptr(projection));

   // Bottom panel vertices
   float vertices[6][2] = {{0.0f, 0.0f},
                           {0.0f, 24.0f},
                           {static_cast<float>(params.width), 0.0f}, //
                                                                     //
                           {0.0f, 24.0f},
                           {static_cast<float>(params.width), 0.0f},
                           {static_cast<float>(params.width), 24.0f}};

   // Draw vertices
   gl.glBindVertexArray(p->vao_);
   gl.glBindBuffer(GL_ARRAY_BUFFER, p->vbo_);
   gl.glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
   gl.glDrawArrays(GL_TRIANGLES, 0, 6);

   // Render time
   p->textShader_.RenderText(plotTimeString,
                             params.width - 7.0f,
                             7.0f,
                             16.0f,
                             projection,
                             boost::gil::rgba8_pixel_t(255, 255, 255, 204),
                             p->font_,
                             p->texture_,
                             gl::TextAlign::Right);

   SCWX_GL_CHECK_ERROR();
}

void OverlayLayer::deinitialize()
{
   gl::OpenGLFunctions& gl = p->gl_;

   BOOST_LOG_TRIVIAL(debug) << logPrefix_ << "deinitialize()";

   gl.glDeleteVertexArrays(1, &p->vao_);
   gl.glDeleteBuffers(1, &p->vbo_);
   gl.glDeleteTextures(1, &p->texture_);

   p->uMVPMatrixLocation_ = GL_INVALID_INDEX;
   p->uColorLocation_     = GL_INVALID_INDEX;
   p->vao_                = GL_INVALID_INDEX;
   p->vbo_                = {GL_INVALID_INDEX};
   p->texture_            = GL_INVALID_INDEX;

   disconnect(p->radarProductView_.get(),
              &view::RadarProductView::PlotUpdated,
              this,
              &OverlayLayer::ReceivePlotUpdate);
}

void OverlayLayer::ReceivePlotUpdate()
{
   p->plotUpdated_ = true;
}

} // namespace map
} // namespace qt
} // namespace scwx
