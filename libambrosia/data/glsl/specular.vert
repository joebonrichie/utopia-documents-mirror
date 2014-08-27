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

/* Vertex shader for specular shading */

/* Interpolated normal */
varying vec3 normal;
/* Lighting terms */
varying vec4 ambient, diffuse, specular;
/* Convenience values */
varying vec3 lightDirection, halfVector;

void main()
{
	/* transform the normal into eye space and normalize the result */
	normal = normalize(gl_NormalMatrix * gl_Normal);

	/* now normalize the light's direction */
	lightDirection = normalize(vec3(gl_LightSource[0].position));

	/* Normalize the halfVector to pass it to the fragment shader */
	halfVector = normalize(gl_LightSource[0].halfVector.xyz);

	/* Compute the diffuse and ambient terms */
	ambient = gl_FrontMaterial.ambient * gl_LightSource[0].ambient;
	ambient += gl_LightModel.ambient * gl_FrontMaterial.ambient;
	diffuse = gl_FrontMaterial.diffuse * gl_LightSource[0].diffuse;
	specular = gl_FrontMaterial.specular * gl_LightSource[0].specular;

	/* Set colour and position */
	gl_FrontColor = gl_Color;
	gl_Position = ftransform();
}