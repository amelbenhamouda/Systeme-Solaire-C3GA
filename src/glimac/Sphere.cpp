#include <cmath>
#include <vector>
#include <iostream>
#include "glimac/common.hpp"
#include "glimac/Sphere.hpp"

#include "c3ga/c3gaTools.hpp"

namespace glimac {
    Sphere::Sphere(const c3ga::Mvec<GLfloat> &radiusVector, GLuint64 latitude, GLuint64 longitude) : 
    radiusVector(radiusVector), latitude(latitude), longitude(longitude) {}
        
    // Constructeur: alloue le tableau de données et construit les attributs des vertex
    Sphere::Sphere(GLfloat radius, GLsizei discLat, GLsizei discLong):
        vertexCount(0) {
        build(radius, discLat, discLong); // Construction (voir le .cpp)
    }

    Sphere::~Sphere(){}

    void Sphere::build(GLfloat r, GLsizei discLat, GLsizei discLong) {
        c3ga::Mvec<double> s = sphere(r);

        c3ga::Mvec<double> ds = s.dual();

        float rs = sqrt(ds * ds) / std::abs(ds[c3ga::E0]) * 2;

        GLfloat rcpLat = 1.f / discLat, rcpLong = 1.f / discLong;
        GLfloat dPhi = 2 * glm::pi<float>() * rcpLat, dTheta = glm::pi<float>() * rcpLong;
        
        std::vector<ShapeVertex> data;
        // Construit l'ensemble des vertex
        for(GLsizei j = 0; j <= discLong; ++j) {
            GLfloat cosTheta = cos(-glm::pi<float>() / 2 + j * dTheta);
            GLfloat sinTheta = sin(-glm::pi<float>() / 2 + j * dTheta);
            
            for(GLsizei i = 0; i <= discLat; ++i) {
                ShapeVertex vertex;
                
                vertex.texCoords.x = i * rcpLat;
                vertex.texCoords.y = 1.f - j * rcpLong;

                vertex.normal.x = sin(i * dPhi) * cosTheta;
                vertex.normal.y = sinTheta;
                vertex.normal.z = cos(i * dPhi) * cosTheta;
                
                vertex.position = rs * vertex.normal;
                
                data.push_back(vertex);
            }
        }

        vertexCount = discLat * discLong * 6;
        for(GLsizei j = 0; j < discLong; ++j) {
            GLsizei offset = j * (discLat + 1);
            for(GLsizei i = 0; i < discLat; ++i) {
                vertices.push_back(data[offset + i]);
                vertices.push_back(data[offset + (i + 1)]);
                vertices.push_back(data[offset + discLat + 1 + (i + 1)]);
                vertices.push_back(data[offset + i]);
                vertices.push_back(data[offset + discLat + 1 + (i + 1)]);
                vertices.push_back(data[offset + i + discLat + 1]);
            }
        }
    }

    c3ga::Mvec<double> Sphere::sphere(float Rsphere) {
        c3ga::Mvec<double> x1, x2, x3, x4, p1, p2, p3, p4;
        c3ga::Mvec<double> dualSphere;
        x1 = (0.0 * c3ga::e1<double>()) + (0.0 * c3ga::e2<double>()) + (Rsphere * c3ga::e3<double>());
        p1 = c3ga::e0<double>() + x1 + (0.5 * x1.quadraticNorm()*c3ga::ei<double>());

        x2 = (0.0 * c3ga::e1<double>()) + (0.0 * c3ga::e2<double>()) + (-Rsphere * c3ga::e3<double>());
        p2 = c3ga::e0<double>() + x2 + (0.5 * x2.quadraticNorm()*c3ga::ei<double>());

        x3 = (Rsphere * c3ga::e1<double>()) + (0.0 * c3ga::e2<double>()) + (0.0 * c3ga::e3<double>());
        p3 = c3ga::e0<double>() + x3 + (0.5*x3.quadraticNorm()*c3ga::ei<double>());

        x4 = (0.0 * c3ga::e1<double>()) + (Rsphere * c3ga::e2<double>()) + (0.0 * c3ga::e3<double>());
        p4 = c3ga::e0<double>() + x4 + (0.5 * x4.quadraticNorm()*c3ga::ei<double>());

        s = (p1 ^ p2 ^ p3 ^ p4);
        dualSphere = s.dual();
        coordsphere.push_back(dualSphere[c3ga::E1] / std::abs(dualSphere[c3ga::E0]));
        coordsphere.push_back(dualSphere[c3ga::E2] / std::abs(dualSphere[c3ga::E0]));
        coordsphere.push_back(dualSphere[c3ga::E3] / std::abs(dualSphere[c3ga::E0]));

        return s;
    }

    const ShapeVertex* Sphere::getDataPointer() const {
        return &vertices[0];
    }
            
    GLsizei Sphere::getVertexCount() const {
        return vertexCount;
    }

    c3ga::Mvec<double> Sphere::getSphere() {
        return s;
    }

    void Sphere::setSphere(c3ga::Mvec<double> sph) {
        s = sph;
    }

    std::list<c3ga::Mvec<double>> Sphere::getCoordsphere() {
        return coordsphere;
    }
}
