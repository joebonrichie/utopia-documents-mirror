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

/* Fragment program for specular shading */

/* Interpolated normal */
varying vec3 normal;
/* Lighting terms */
varying vec4 ambient, diffuse, specular;
/* Convenience values */
varying vec3 lightDirection, halfVector;

void main()
{
	vec3 n, halfV, ldir;
	float NdotL, NdotHV;

	/* The ambient term will always be present */
	vec4 color = ambient * gl_Color;

	/* Normalised interpolated normal / light direction */
	n = normalize(normal);
	ldir = normalize(lightDirection);

	/* Compute the dot product between normal and lightDirection */
	NdotL = max(dot(n, ldir), 0.0);

	/* Compute diffuse and specular */
	if (NdotL > 0.0) {
		color += diffuse * NdotL * gl_Color;
		halfV = normalize(halfVector);
		NdotHV = max(dot(n, halfV), 0.0);
		color += specular * pow(NdotHV, gl_FrontMaterial.shininess);
	}

	gl_FragColor = color;
}
