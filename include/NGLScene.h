#ifndef NGLSCENE_H_
#define NGLSCENE_H_
#include "WindowParams.h"
#include <ngl/Camera.h>
#include <ngl/Text.h>
#include <ngl/Transformation.h>
#include <ngl/Obj.h>
#include <QOpenGLWindow>
#include <CubeMap.h>

//----------------------------------------------------------------------------------------------------------------------
/// @file NGLScene.h
/// @brief this class inherits from the Qt OpenGLWindow and allows us to use NGL to draw OpenGL
/// @author Jonathan Macey
/// @version 1.0
/// @date 10/9/13
/// Revision History :
/// This is an initial version used for the new NGL6 / Qt 5 demos
/// @class NGLScene
/// @brief our main glwindow widget for NGL applications all drawing elements are
/// put in this file
//----------------------------------------------------------------------------------------------------------------------

class NGLScene : public QOpenGLWindow
{
  Q_OBJECT
public:
  //----------------------------------------------------------------------------------------------------------------------
  /// @brief ctor for our NGL drawing class
  /// @param [in] parent the parent window to the class
  //----------------------------------------------------------------------------------------------------------------------
  NGLScene();
  //----------------------------------------------------------------------------------------------------------------------
  /// @brief dtor must close down ngl and release OpenGL resources
  //----------------------------------------------------------------------------------------------------------------------
  ~NGLScene();
  //----------------------------------------------------------------------------------------------------------------------
  /// @brief the initialize class is called once when the window is created and we have a valid GL context
  /// use this to setup any default GL stuff
  //----------------------------------------------------------------------------------------------------------------------
  void initializeGL() override;
  //----------------------------------------------------------------------------------------------------------------------
  /// @brief this is called everytime we want to draw the scene
  //----------------------------------------------------------------------------------------------------------------------
  void paintGL() override;
  //----------------------------------------------------------------------------------------------------------------------
  /// @brief this is called everytime we want to draw the scene
  //----------------------------------------------------------------------------------------------------------------------
  void resizeGL(int _w, int _h) override;


  bool loadImage(const std::string  &_fname );

private:
  //----------------------------------------------------------------------------------------------------------------------
  /// @brief the windows params such as mouse and rotations etc
  //----------------------------------------------------------------------------------------------------------------------
  WinParams m_win;
  //----------------------------------------------------------------------------------------------------------------------
  /// @brief used to store the global mouse transforms
  //----------------------------------------------------------------------------------------------------------------------
  ngl::Mat4 m_mouseGlobalTX;
  //----------------------------------------------------------------------------------------------------------------------
  /// @brief Our Camera
  //------------------------------------ //----------------------------------------------------------------------------------------------------------------------
  ngl::Camera m_cam;
  //----------------------------------------------------------------------------------------------------------------------
  /// @brief the model position for mouse movement
  //----------------------------------------------------------------------------------------------------------------------
  ngl::Vec3 m_modelPos;
  //----------------------------------------------------------------------------------------------------------------------
  /// @brief the model transformation
  //----------------------------------------------------------------------------------------------------------------------
  ngl::Transformation m_transform;
  //----------------------------------------------------------------------------------------------------------------------
  /// @brief method to load transform matrices to the shader
  //----------------------------------------------------------------------------------------------------------------------
  void loadMatricesToShader();
  void loadMatricesToShader2();
  void loadMatricesToShader3();
  //----------------------------------------------------------------------------------------------------------------------
  /// @brief Qt Event called when a key is pressed
  /// @param [in] _event the Qt event to query for size etc
  //----------------------------------------------------------------------------------------------------------------------
  void keyPressEvent(QKeyEvent *_event) override;
  //----------------------------------------------------------------------------------------------------------------------
  /// @brief this method is called every time a mouse is moved
  /// @param _event the Qt Event structure
  //----------------------------------------------------------------------------------------------------------------------
  void mouseMoveEvent(QMouseEvent *_event) override;
  //----------------------------------------------------------------------------------------------------------------------
  /// @brief this method is called everytime the mouse button is pressed
  /// inherited from QObject and overridden here.
  /// @param _event the Qt Event structure
  //----------------------------------------------------------------------------------------------------------------------
  void mousePressEvent(QMouseEvent *_event) override;
  //----------------------------------------------------------------------------------------------------------------------
  /// @brief this method is called everytime the mouse button is released
  /// inherited from QObject and overridden here.
  /// @param _event the Qt Event structure
  //----------------------------------------------------------------------------------------------------------------------
  void mouseReleaseEvent(QMouseEvent *_event) override;
  //----------------------------------------------------------------------------------------------------------------------
  /// @brief this method is called everytime the mouse wheel is moved
  /// inherited from QObject and overridden here.
  /// @param _event the Qt Event structure
  //----------------------------------------------------------------------------------------------------------------------
  void wheelEvent(QWheelEvent *_event) override;

  std::unique_ptr<ngl::Obj> m_mesh, m_mesh2;


  void initTexture(const GLuint& /*texUnit*/, GLuint &/*texId*/, const char */*filename*/);

  /// Initialise the entire environment map
  void initEnvironment();

  /// Initialise a single side of the environment map
  void initEnvironmentSide(GLenum /*target*/, const char* /*filename*/);

  //void buildVAO();

  GLuint m_envTex, m_glossMapTex, m_textMap, m_envTex2, m_renderedTex2, m_normalMap, m_metallicMap, m_roughnessMap, m_aoMap;


  std::unique_ptr <CubeMap> m_cubeMap;
  void createSkyBox();

  std::unique_ptr<ngl::AbstractVAO> m_skybox;

  unsigned int loadCubemap();


};


#endif
