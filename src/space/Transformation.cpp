#include <cmath>
#include <list>
#include <iostream>
#include "glimac/common.hpp"
#include "c3ga/c3gaTools.hpp"
#include "space/Transformation.hpp"

c3ga::Mvec<double> Transformation::translate(c3ga::Mvec<double> vect, double facteur, c3ga::Mvec<double> translation) {
    c3ga::Mvec<double> translator = 1 - 0.5 * translation * c3ga::ei<double>() * facteur;
	vect = translator * vect * translator.inv();
	return vect;
}

glm::vec3 Transformation::applyTranslationX(Sphere sphere) {
	c3ga::Mvec<double> s = sphere.getSphere();
	return glm::vec3(abs(s[c3ga::E0123]) + abs(s[c3ga::E123i]) + abs(s[c3ga::E0123i]), 0, 0);
}

c3ga::Mvec<double> Transformation::rotate(c3ga::Mvec<double> vect, double angle, c3ga::Mvec<double> biVect) {
    c3ga::Mvec<double> rotor = cos(0.5 * angle * M_PI / 180) - biVect * sin(0.5 * angle * M_PI / 180);    
	vect = rotor * vect * rotor.inv();
	vect.roundZero(1.0e-10);
    return vect;
}

glm::vec3 Transformation::applyRotation(Sphere sphere) {
	c3ga::Mvec<double> s = sphere.getSphere();
	return glm::vec3(s[c3ga::E0123], s[c3ga::E0123], s[c3ga::E0123]);
}

c3ga::Mvec<double> Transformation::scale(c3ga::Mvec<double> vect, double scale) {
    c3ga::Mvec<double> dilator = 1. - ((1. - scale) / (1. + scale)) * c3ga::e0i<double>();  
	vect = dilator * vect * dilator.inv();
	vect.roundZero(1.0e-10);
	return vect;
}

glm::vec3 Transformation::applyScale(Sphere sphere) {
	c3ga::Mvec<double> s = sphere.getSphere();
	return glm::vec3(s[c3ga::E0123], s[c3ga::E0123], s[c3ga::E0123]);
}