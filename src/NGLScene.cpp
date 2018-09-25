#include "NGLScene.h"
#include <QGuiApplication>
#include <QMouseEvent>

#include <ngl/Camera.h>
#include <ngl/NGLInit.h>
#include <ngl/NGLStream.h>
#include <ngl/Random.h>
#include <ngl/ShaderLib.h>
#include <ngl/VAOPrimitives.h>
#include <ngl/VAOFactory.h>
#include <ngl/SimpleVAO.h>
#include <ngl/Obj.h>
#include <ngl/Texture.h>

#include <ngl/VAOFactory.h>

#include <ngl/SimpleIndexVAO.h>
#include "CubeMap.h"

// This demo is based on code from here https://learnopengl.com/#!PBR/Lighting
NGLScene::NGLScene()
{
  setTitle( "PBR with GLSL" );
}



NGLScene::~NGLScene()
{
  std::cout << "Shutting down NGL, removing VAO's and Shaders\n";
}



void NGLScene::resizeGL( int _w, int _h )
{
  m_cam.setShape( 45.0f, static_cast<float>( _w ) / _h, 0.05f, 350.0f );
  m_win.width  = static_cast<int>( _w * devicePixelRatio() );
  m_win.height = static_cast<int>( _h * devicePixelRatio() );
}


// lights
std::array<ngl::Vec3,8> g_lightPositions = {{
        ngl::Vec3(-7.0f,  0.0f, -7.0f),
        ngl::Vec3( 7.0f,  0.0f, -7.0f),
        ngl::Vec3(-7.0f,  0.0f, 7.0f),
        ngl::Vec3( 7.0f,  0.0f, 7.0f),
        ngl::Vec3(-6.0f, 0.0f, -2.0f),
        ngl::Vec3(2.0f, 0.0f, -6.0f),
        ngl::Vec3(-2.0f, 0.0f, 6.0f),
        ngl::Vec3(6.0f, 0.0f, 2.0f)
    }};




void NGLScene::initializeGL()
{
  // we must call that first before any other GL commands to load and link the
  // gl commands from the lib, if that is not done program will crash
  ngl::NGLInit::instance();


  glClearColor( 0.4f, 0.4f, 0.4f, 1.0f ); // Grey Background
  // enable depth testing for drawing
  glEnable( GL_DEPTH_TEST );
  // enable multisampling for smoother drawing
#ifndef USINGIOS_
  glEnable( GL_MULTISAMPLE );
#endif

  ngl::ShaderLib* shaderF = ngl::ShaderLib::instance();
  constexpr auto shaderFloor = "Floor";
  constexpr auto vertexFloor  = "PBRVertexF";
  constexpr auto fragFloor    = "PBRFragmentF";

  // create the shader program for FLOOR
  shaderF->createShaderProgram( shaderFloor );
  // now we are going to create empty shaders for Frag and Vert
  shaderF->attachShader( vertexFloor, ngl::ShaderType::VERTEX );
  shaderF->attachShader( fragFloor, ngl::ShaderType::FRAGMENT );
  // attach the source
  shaderF->loadShaderSource( vertexFloor, "/home/raluca/Documents/cpp/rendering/MetalPBR/shaders/myPBRvertex.glsl" );
  shaderF->loadShaderSource( fragFloor, "/home/raluca/Documents/cpp/rendering/MetalPBR/shaders/myPBRfragment.glsl" );
  // compile the shaders
  shaderF->compileShader( vertexFloor );
  shaderF->compileShader( fragFloor );
  // add them to the program
  shaderF->attachShaderToProgram( shaderFloor, vertexFloor );
  shaderF->attachShaderToProgram( shaderFloor, fragFloor );
  // now we have associated that data we can link the shader
  shaderF->linkProgramObject( shaderFloor );



  // now to load the shader and set the values
  // grab an instance of shader manager
  ngl::ShaderLib* shader = ngl::ShaderLib::instance();
  // we are creating a shader called PBR to save typos
  // in the code create some constexpr
  constexpr auto shaderProgram = "PBR";
  constexpr auto vertexShader  = "PBRVertex";
  constexpr auto fragShader    = "PBRFragment";

  // create the shader program
  shader->createShaderProgram( shaderProgram );
  // now we are going to create empty shaders for Frag and Vert
  shader->attachShader( vertexShader, ngl::ShaderType::VERTEX );
  shader->attachShader( fragShader, ngl::ShaderType::FRAGMENT );
  // attach the source
  shader->loadShaderSource( vertexShader, "/home/raluca/Documents/cpp/rendering/MetalPBR/shaders/PBRVertex.glsl" );
  shader->loadShaderSource( fragShader, "/home/raluca/Documents/cpp/rendering/MetalPBR/shaders/PBRFragment.glsl" );
  // compile the shaders
  shader->compileShader( vertexShader );
  shader->compileShader( fragShader );
  // add them to the program
  shader->attachShaderToProgram( shaderProgram, vertexShader );
  shader->attachShaderToProgram( shaderProgram, fragShader );
  // now we have associated that data we can link the shader
  shader->linkProgramObject( shaderProgram );



  // Create and compile the vertex and fragment shader
  //FOR ENVIRONMENT MAP / CUBE MAP

  ngl::ShaderLib* shaderE=ngl::ShaderLib::instance();

  shaderE->loadShader("EnvironmentProgram",
                      "shaders/PBRVertex.glsl",
                      "shaders/EnvFragShader.glsl");

  // Initialise our environment map here
  initEnvironment();

  // Initialise our gloss texture map here
  initTexture(1, m_glossMapTex, "images/gloss.png");
  shaderE->use("EnvironmentProgram");
  shaderE->setUniform("glossMap", 1);


  // and make it active ready to load values
  ( *shader )[ "PBR" ]->use();


  // Now we will create a basic Camera from the graphics library
  // This is a static camera so it only needs to be set once
  // First create Values for the camera position
  ngl::Vec3 from( 0, 5, 13 );
  ngl::Vec3 to( 0, 0, 0 );
  ngl::Vec3 up( 0, 1, 0 );

  // now load to our new camera
  m_cam.set( from, to, up );
  // set the shape using FOV 45 Aspect Ratio based on Width and Height
  // The final two are near and far clipping planes of 0.5 and 10
  m_cam.setShape( 45.0f, 720.0f / 576.0f, 0.05f, 350.0f );


  shader->setUniform("albedo",1.0f, 0.0f, 0.0f);
  shader->setUniform("ao",1.0f);
  shader->setUniform("roughness", 1.0f);
  shader->setUniform("metallic", 0.0f);


  shader->setUniform("camPos",m_cam.getEye().toVec3());
  shader->setUniform("exposure",1.0f);

  m_mesh.reset(new ngl::Obj("data/bottleOpener.obj"));
  m_mesh->createVAO();


  ///ARRAY OF LIGHT COLORS
  ///

  std::array<ngl::Vec3,8>  lightColors = {{
          ngl::Vec3(300.0f, 300.0f, 300.0f),
          ngl::Vec3(300.0f, 300.0f, 300.0f),
          ngl::Vec3(300.0f, 300.0f, 300.0f),
          ngl::Vec3(300.0f, 300.0f, 300.0f),
          ngl::Vec3(300.0f, 300.0f, 300.0f),
          ngl::Vec3(300.0f, 300.0f, 300.0f),
          ngl::Vec3(300.0f, 300.0f, 300.0f),
          ngl::Vec3(300.0f, 300.0f, 300.0f)
      }};

  for(size_t i=0; i<g_lightPositions.size(); ++i)
  {
    shader->setUniform(("lightPositions[" + std::to_string(i) + "]").c_str(),g_lightPositions[i]);
    shader->setUniform(("lightColors[" + std::to_string(i) + "]").c_str(),lightColors[i]);
  }

  ( *shader )[ ngl::nglColourShader ]->use();
  shader->setUniform("Colour",1.0f,1.0f,1.0f,1.0f);


  ///NGL PRIMITIVES FOR LIGHT AND FLOOR
  ngl::VAOPrimitives::instance()->createSphere("sphere",1.0,30.0f);
  ngl::VAOPrimitives::instance()->createTrianglePlane("floor",20,20,10,10,ngl::Vec3::up());

}





void NGLScene::paintGL()
{

  //DRAW THE SKYBOX
  /*float skyboxVertices[] = {
      // positions
      -15.0f,  15.0f, -15.0f,
      -15.0f, -15.0f, -15.0f,
       15.0f, -15.0f, -15.0f,
       15.0f, -15.0f, -15.0f,
       15.0f,  15.0f, -15.0f,
      -15.0f,  15.0f, -15.0f,

      -15.0f, -15.0f,  15.0f,
      -15.0f, -15.0f, -15.0f,
      -15.0f,  15.0f, -15.0f,
      -15.0f,  15.0f, -15.0f,
      -15.0f,  15.0f,  15.0f,
      -15.0f, -15.0f,  15.0f,

       15.0f, -15.0f, -15.0f,
       15.0f, -15.0f,  15.0f,
       15.0f,  15.0f,  15.0f,
       15.0f,  15.0f,  15.0f,
       15.0f,  15.0f, -15.0f,
       15.0f, -15.0f, -15.0f,

      -15.0f, -15.0f,  15.0f,
      -15.0f,  15.0f,  15.0f,
       15.0f,  15.0f,  15.0f,
       15.0f,  15.0f,  15.0f,
       15.0f, -15.0f,  15.0f,
      -15.0f, -15.0f,  15.0f,

      -15.0f,  15.0f, -15.0f,
       15.0f,  15.0f, -15.0f,
       15.0f,  15.0f,  15.0f,
       15.0f,  15.0f,  15.0f,
      -15.0f,  15.0f,  15.0f,
      -15.0f,  15.0f, -15.0f,

      -15.0f, -15.0f, -15.0f,
      -15.0f, -15.0f,  15.0f,
       15.0f, -15.0f, -15.0f,
       15.0f, -15.0f, -15.0f,
      -15.0f, -15.0f,  15.0f,
       15.0f, -15.0f,  15.0f
  };*/

  glViewport( 0, 0, m_win.width, m_win.height );
  // clear the screen and depth buffer
  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );


  /*ngl::ShaderLib* shaderE = ngl::ShaderLib::instance();
  ( *shaderE )[ "EnvironmentProgram" ]->use();

  // skybox VAO
  unsigned int skyboxVAO;
  glGenVertexArrays(1, &skyboxVAO);
  //glGenBuffers(1, &skyboxVBO);
  glBindVertexArray(skyboxVAO);
  //glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);


  // draw skybox as last

  loadMatricesToShader2();
  // skybox cube
  glBindVertexArray(skyboxVAO);
  initEnvironment();
  glDrawArrays(GL_TRIANGLES, 0, 36);
  glBindVertexArray(0);*/


  // Rotation based on the mouse position for our global transform
  ngl::Mat4 rotX;
  ngl::Mat4 rotY;
  // create the rotation matrices
  rotX.rotateX( m_win.spinXFace );
  rotY.rotateY( m_win.spinYFace );
  // multiply the rotations
  m_mouseGlobalTX = rotX * rotY;
  // add the translations
  m_mouseGlobalTX.m_m[ 3 ][ 0 ] = m_modelPos.m_x;
  m_mouseGlobalTX.m_m[ 3 ][ 1 ] = m_modelPos.m_y;
  m_mouseGlobalTX.m_m[ 3 ][ 2 ] = m_modelPos.m_z;


  // grab an instance of the shader manager
  ngl::ShaderLib* shader = ngl::ShaderLib::instance();
  ( *shader )[ "PBR" ]->use();

  // get the VBO instance and draw the built in teapot
  ngl::VAOPrimitives* prim = ngl::VAOPrimitives::instance();

  m_transform.setPosition(0.0f, 0.0f, 0.0f);
  loadMatricesToShader();
  //m_mesh->draw();
  prim->draw("sphere");



  // draw floor
  //shader->setUniform("albedo",0.1f, 0.1f, 0.1f);
  ngl::ShaderLib* shaderF = ngl::ShaderLib::instance();
  ( *shaderF )[ "Floor" ]->use();
  shader->setUniform("albedo", 0.3f, 0.1f, 0.2f);
  shader->setUniform("ao",1.0f);
  shader->setUniform("roughness",0.2f);
  shader->setUniform("metallic", 0.7f);
  m_transform.reset();
  m_transform.setPosition(0.0f,-0.5f,0.0f);
  loadMatricesToShader();
  prim->draw("floor");


  // Draw Lights
  ( *shader )[ ngl::nglColourShader ]->use();
  ngl::Mat4 MVP;
  ngl::Transformation tx;
  shader->setUniform("Colour",1.0f,1.0f,1.0f,1.0f);

  for(size_t i=0; i<g_lightPositions.size(); ++i)
  {
    tx.setPosition(g_lightPositions[i]);
    MVP=m_cam.getVPMatrix()* m_mouseGlobalTX * tx.getMatrix() ;
    shader->setUniform("MVP",MVP);
    prim->draw("sphere");
  }

}


void NGLScene::initTexture(const GLuint& texUnit, GLuint &texId, const char *filename) {
    // Set our active texture unit
    glActiveTexture(GL_TEXTURE0 + texUnit);

    // Load up the image using NGL routine
    ngl::Image img(filename);

    // Create storage for our new texture
    glGenTextures(1, &texId);

    // Bind the current texture
    glBindTexture(GL_TEXTURE_2D, texId);

    // Transfer image data onto the GPU using the teximage2D call
    glTexImage2D (
                GL_TEXTURE_2D,    // The target (in this case, which side of the cube)
                0,                // Level of mipmap to load
                img.format(),     // Internal format (number of colour components)
                img.width(),      // Width in pixels
                img.height(),     // Height in pixels
                0,                // Border
                GL_RGB,          // Format of the pixel data
                GL_UNSIGNED_BYTE, // Data type of pixel data
                img.getPixels()); // Pointer to image data in memory

    // Set up parameters for our texture
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
}

/**
 * @brief Scene::initEnvironment in texture unit 0
 */
void NGLScene::initEnvironment() {
    // Enable seamless cube mapping
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

    // Placing our environment map texture in texture unit 0
    glActiveTexture (GL_TEXTURE0);

    // Generate storage and a reference for our environment map texture
    glGenTextures (1, &m_envTex);

    // Bind this texture to the active texture unit
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_envTex);

    // Now load up the sides of the cube
    initEnvironmentSide(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, "images/sky_zneg.png");
    initEnvironmentSide(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, "images/sky_zpos.png");
    initEnvironmentSide(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, "images/sky_ypos.png");
    initEnvironmentSide(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, "images/sky_yneg.png");
    initEnvironmentSide(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, "images/sky_xneg.png");
    initEnvironmentSide(GL_TEXTURE_CUBE_MAP_POSITIVE_X, "images/sky_xpos.png");

    // Generate mipmap levels
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

    // Set the texture parameters for the cube map
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_AUTO_GENERATE_MIPMAP, GL_TRUE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    GLfloat anisotropy;
    glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &anisotropy);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_ANISOTROPY_EXT, anisotropy);

    // Set our cube map texture to on the shader so we can use it
    ngl::ShaderLib *shader=ngl::ShaderLib::instance();
    shader->use("EnvironmentProgram");
    shader->setUniform("envMap", 0);
}

/**
 * @brief Scene::initEnvironmentSide
 * @param texture
 * @param target
 * @param filename
 * This function should only be called when we have the environment texture bound already
 * copy image data into 'target' side of cube map
 */
void NGLScene::initEnvironmentSide(GLenum target, const char *filename) {
    // Load up the image using NGL routine
    ngl::Image img(filename);

    // Transfer image data onto the GPU using the teximage2D call
    glTexImage2D (
      target,           // The target (in this case, which side of the cube)
      0,                // Level of mipmap to load
      img.format(),     // Internal format (number of colour components)
      img.width(),      // Width in pixels
      img.height(),     // Height in pixels
      0,                // Border
      GL_RGBA,          // Format of the pixel data
      GL_UNSIGNED_BYTE, // Data type of pixel data
      img.getPixels()   // Pointer to image data in memory
    );
}

void NGLScene::loadMatricesToShader()
{
  ngl::ShaderLib* shader = ngl::ShaderLib::instance();

  ngl::Mat4 MV;
  ngl::Mat4 MVP;
  ngl::Mat3 normalMatrix;
  ngl::Mat4 M;
  M            = m_mouseGlobalTX * m_transform.getMatrix() ;
  MV           = m_cam.getViewMatrix() * M;
  MVP          = m_cam.getVPMatrix() * M;

  normalMatrix = MV;
  normalMatrix.inverse().transpose();
  shader->setUniform( "MVP", MVP );
  shader->setUniform( "normalMatrix", normalMatrix );
  shader->setUniform( "M", M );
}

void NGLScene::loadMatricesToShader2()
{
  ngl::ShaderLib* shader = ngl::ShaderLib::instance();

  ngl::Mat4 MV;
  ngl::Mat4 MVP;
  ngl::Mat3 normalMatrix;
  ngl::Mat4 M;
  ngl::Mat4 view = ngl::Mat4(m_cam.getViewMatrix());
  M            = m_mouseGlobalTX * m_transform.getMatrix() ;
  MV           = view * M;
  MVP          = m_cam.getVPMatrix() * M;

  normalMatrix = MV;
  normalMatrix.inverse().transpose();
  shader->setUniform( "MVP", MVP );
  shader->setUniform( "normalMatrix", normalMatrix );
  shader->setUniform( "M", M );
}


/*unsigned int loadCubemap(vector<std::string> faces)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++)
    {
        unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                         0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
            );
            stbi_image_free(data);
        }
        else
        {
            std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}*/

//----------------------------------------------------------------------------------------------------------------------

void NGLScene::keyPressEvent( QKeyEvent* _event )
{
  // that method is called every time the main window recives a key event.
  // we then switch on the key value and set the camera in the GLWindow
  switch ( _event->key() )
  {
    // escape key to quit
    case Qt::Key_Escape:
      QGuiApplication::exit( EXIT_SUCCESS );
      break;
// turn on wirframe rendering
#ifndef USINGIOS_
    case Qt::Key_W:
      glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
      break;
    // turn off wire frame
    case Qt::Key_S:
      glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
      break;
#endif
    // show full screen
    case Qt::Key_F:
      showFullScreen();
      break;
    // show windowed
    case Qt::Key_N:
      showNormal();
      break;
    case Qt::Key_Space :
      m_win.spinXFace=0;
      m_win.spinYFace=0;
      m_modelPos.set(ngl::Vec3::zero());
    break;
    default:
      break;
  }
  update();
}
