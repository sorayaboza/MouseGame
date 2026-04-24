#include "_skybox.h"

_skyBox::_skyBox() {
    //ctor
    rot = {0,0,0};
    scale = {100,150,100};
    pos = {0,0,0};
}

_skyBox::~_skyBox() { /*dtor*/ }

void _skyBox::drawBox() {
     glDisable(GL_LIGHTING);
     glPushMatrix();
       glColor3f(0.95f, 0.9f, 0.85f);
       glTranslatef(pos.x,pos.y,pos.z);
       glRotatef(rot.x,1,0,0);
       glRotatef(rot.y,0,1,0);
       glRotatef(rot.z,0,0,1);

       glScalef(scale.x,scale.y,scale.z);

       glEnableClientState(GL_VERTEX_ARRAY);
       glEnableClientState(GL_NORMAL_ARRAY);
       glEnableClientState(GL_TEXTURE_COORD_ARRAY);

       glBindBuffer(GL_ARRAY_BUFFER,vboTex);
       glTexCoordPointer(2,GL_FLOAT,0,(void*)0);

       glBindBuffer(GL_ARRAY_BUFFER,vboNorm);
       glNormalPointer(GL_FLOAT,0,0);

       glBindBuffer(GL_ARRAY_BUFFER,vboVert);
       glVertexPointer(3,GL_FLOAT,0,0);

       glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,eboID);
       myTex[0].BindTex();
       glDrawElements(GL_TRIANGLES,6,GL_UNSIGNED_INT,0); // front face
       myTex[1].BindTex();
       glDrawElements(GL_TRIANGLES,6,GL_UNSIGNED_INT,(void*)(6*sizeof(unsigned int))); //back face
       myTex[2].BindTex();
       //glDrawElements(GL_TRIANGLES,6,GL_UNSIGNED_INT,(void*)(12*sizeof(unsigned int)));//top face
       //myTex[3].BindTex();
       glDrawElements(GL_TRIANGLES,6,GL_UNSIGNED_INT,(void*)(18*sizeof(unsigned int)));//bottom face
       myTex[4].BindTex();
       glDrawElements(GL_TRIANGLES,6,GL_UNSIGNED_INT,(void*)(24*sizeof(unsigned int)));//left face
       myTex[5].BindTex();
       glDrawElements(GL_TRIANGLES,6,GL_UNSIGNED_INT,(void*)(30*sizeof(unsigned int)));//right face

       glDisableClientState(GL_VERTEX_ARRAY);
       glDisableClientState(GL_NORMAL_ARRAY);
       glDisableClientState(GL_TEXTURE_COORD_ARRAY);

       glEnable(GL_LIGHTING);
    glPopMatrix();
}

void _skyBox::boxInit() {
      //vertex Buffer
    glGenBuffers(1,&vboVert);
    glBindBuffer(GL_ARRAY_BUFFER,vboVert);
    glBufferData(GL_ARRAY_BUFFER,sizeof(vertices),vertices,GL_STATIC_DRAW);

     //Normal Buffer
    glGenBuffers(1,&vboNorm);
    glBindBuffer(GL_ARRAY_BUFFER,vboNorm);
    glBufferData(GL_ARRAY_BUFFER,sizeof(normals),normals,GL_STATIC_DRAW);

     //texture Buffer
    glGenBuffers(1,&vboTex);
    glBindBuffer(GL_ARRAY_BUFFER,vboTex);
    glBufferData(GL_ARRAY_BUFFER,sizeof(texCoords),texCoords,GL_STATIC_DRAW);

    //index buffer
    glGenBuffers(1,&eboID);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,eboID);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,sizeof(indices),indices,GL_STATIC_DRAW);

    // Unbind buffers after creation
    glBindBuffer(GL_ARRAY_BUFFER,0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0);
}
