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

/* Vertex program for geometric billboarding of spheres */

/* Orientation information */
varying vec3 orientation;
/* Lighting terms */
varying vec4 ambient, diffuse, specular;
/* Convenience values */
varying vec3 lightDirection, halfVector;

void main()
{
	/* Get the radius */
	float radius = gl_Normal.z;

	/* Find coordinates of vertex in eye coordinates */
	vec4 position = gl_ModelViewMatrix * gl_Vertex;
	position.x += gl_Normal.x * radius;
	position.y += gl_Normal.y * radius;

	/* Calculate depth protrusion */
	vec4 protrusion = gl_ModelViewMatrix * gl_Vertex;
	protrusion.z += radius;
	protrusion = gl_ProjectionMatrix * protrusion;

	/* now normalize the light's direction */
	lightDirection = normalize(vec3(gl_LightSource[0].position));

	/* Normalize the halfVector to pass it to the fragment shader */
	halfVector = normalize(gl_LightSource[0].halfVector.xyz);

	/* Pass on orientation information, ensuring application of viewport transformation of NDC.z */
	orientation = vec3(gl_Normal.xy, (protrusion.z / protrusion.w) / 2.0 + 0.5);

	/* compute diffuse */
	ambient = gl_FrontMaterial.ambient * gl_LightSource[0].ambient;
	ambient += gl_LightModel.ambient * gl_FrontMaterial.ambient;
	diffuse = gl_FrontMaterial.diffuse * gl_LightSource[0].diffuse;
	specular = gl_FrontMaterial.specular * gl_LightSource[0].specular;

	/* Pass through this vertex's position */
	gl_Position = gl_ProjectionMatrix * position;

	/* Set colour */
	gl_FrontColor = gl_Color;
}
