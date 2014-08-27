/*****************************************************************************
 *  
 *   This file is part of the Utopia Documents application.
 *       Copyright (c) 2008-2014 Lost Island Labs
 *           <info@utopiadocs.com>
 *   
 *   Utopia Documents is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU GENERAL PUBLIC LICENSE VERSION 3 as
 *   published by the Free Software Foundation.
 *   
 *   Utopia Documents is distributed in the hope that it will be useful, but
 *   WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 *   Public License for more details.
 *   
 *   In addition, as a special exception, the copyright holders give
 *   permission to link the code of portions of this program with the OpenSSL
 *   library under certain conditions as described in each individual source
 *   file, and distribute linked combinations including the two.
 *   
 *   You must obey the GNU General Public License in all respects for all of
 *   the code used other than OpenSSL. If you modify file(s) with this
 *   exception, you may extend this exception to your version of the file(s),
 *   but you are not obligated to do so. If you do not wish to do so, delete
 *   this exception statement from your version.
 *   
 *   You should have received a copy of the GNU General Public License
 *   along with Utopia Documents. If not, see <http://www.gnu.org/licenses/>
 *  
 *****************************************************************************/

/* Fragment program for gemoetric billboarding of spheres */

/* Orientation information */
varying vec3 orientation;
/* Lighting terms */
varying vec4 ambient, diffuse, specular;
/* Convenience values */
varying vec3 lightDirection, halfVector;

void main()
{
	vec3 halfV;
	float NdotL, NdotHV;

	/* Unwrap orientation */
	float x = orientation.x;
	float y = orientation.y;
	float r = gl_FragCoord.z - orientation.z;

	if ((x*x + y*y) > 1.0) discard;

	/* Calculate protrusion */
	float z = sqrt(1.0 - x*x - y*y);

	/* Calculate new depth */
	gl_FragDepth = gl_FragCoord.z - z * r;

	/* Calculate normal */
	vec3 normal = normalize(vec3(x, y, z));

	/* Calculate lighting */
	vec3 ldir = normalize(lightDirection);

	/* Compute the dot product between normal and lightDirection */
	NdotL = max(dot(normal, ldir), 0.0);

	/* Compute diffuse */
	vec4 color = ambient * gl_Color;
	halfV = normalize(halfVector);
	NdotHV = max(dot(normal, halfVector), 0.0);
	vec4 color2 = color + diffuse * NdotL * gl_Color + specular * pow(NdotHV, gl_FrontMaterial.shininess);		
	if (NdotL > 0.0) {
		color = color2;
	}
	
	color.a = gl_Color.a;
	gl_FragColor = color;
}


