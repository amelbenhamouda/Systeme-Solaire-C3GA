#include <cmath>
#include <vector>
#include <iostream>
#include "glimac/common.hpp"
#include "glimac/Tore.hpp"

#include "c3ga/c3gaTools.hpp"

namespace glimac {
    // Constructeur: alloue le tableau de données et construit les attributs des vertex
    Tore::Tore(GLfloat ri, GLfloat re, GLsizei nbi, GLsizei nbe):
        vertexCount(0) {
        build(ri, re, nbi, nbe); // Construction (voir le .cpp)
    }

    Tore::~Tore(){}

    /*
    ri : rayon interieur 
    re : rayon exterieur
    */
    void Tore::build(GLfloat ri, GLfloat re, GLsizei nbi, GLsizei nbe) {
        
        std::vector<ShapeVertex> data;
        float x1, y1, z1;
        float x2, y2, z2;
        float alphai, alphaj;
        float cosalphai, cosalphaj;
        float sinalphai, sinalphaj;
        float beta, cosbeta, sinbeta;
        float fact = 0.1;
        // Construit l'ensemble des vertex
        for (int i = 0; i < nbi + 1; i++) {
            alphai = 2 * M_PI * i / nbi;
            alphaj = alphai + 2 * M_PI / nbi;
            cosalphai = cos(alphai);
            sinalphai = sin(alphai);
            cosalphaj = cos(alphaj);
            sinalphaj = sin(alphaj);
            for (int j = 0; j <= nbe; j++) {
                beta = 2 * M_PI * j / nbe;
                cosbeta = cos(beta);
                sinbeta = sin(beta);

                ShapeVertex vertex;
                x1 = (re + ri * cosbeta) * cosalphai;
                y1 = (re + ri * cosbeta) * sinalphai;
                z1 = ri ;//* sinbeta;
                vertex.texCoords.x = i / nbi * fact;
                vertex.texCoords.y = j / nbe * fact;
                vertex.normal.x = cosbeta * cosalphai;
                vertex.normal.y = cosbeta * sinalphai;
                vertex.normal.z = sinbeta;
                vertex.position.x = x1;
                vertex.position.y = y1;
                vertex.position.z = z1;
                data.push_back(vertex);

                // ShapeVertex vertex2;
                // x2 = (re + ri * cosbeta) * cosalphaj;
                // y2 = (re + ri * cosbeta) * sinalphaj;
                // z2 = ri * sinbeta;
                // vertex2.texCoords.x = (i + 1) / nbi * fact;
                // vertex2.texCoords.y = j / nbe * fact;
                // vertex2.normal.x = cosbeta * cosalphaj;
                // vertex2.normal.y = cosbeta * sinalphaj;
                // vertex2.normal.z = sinbeta;
                // vertex2.position.x = x2;
                // vertex2.position.y = y2;
                // vertex2.position.z = z2;
                // data.push_back(vertex2);
            }

        }

        vertexCount = nbi * nbe * 6;
        for(GLsizei j = 0; j < nbe; ++j) {
            GLsizei offset = j * (nbi + 1);
            for(GLsizei i = 0; i < nbi; ++i) {
                vertices.push_back(data[offset + i]);
                vertices.push_back(data[offset + (i + 1)]);
                vertices.push_back(data[offset + nbi + 1 + (i + 1)]);
                vertices.push_back(data[offset + i]);
                vertices.push_back(data[offset + nbi + 1 + (i + 1)]);
                vertices.push_back(data[offset + i + nbi + 1]);
            }
        }
    }

    const ShapeVertex* Tore::getDataPointer() const {
        return &vertices[0];
    }
            
    GLsizei Tore::getVertexCount() const {
        return vertexCount;
    }
}