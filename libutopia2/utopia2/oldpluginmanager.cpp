/*****************************************************************************
 *  
 *   This file is part of the Utopia Documents application.
 *       Copyright (c) 2008-2017 Lost Island Labs
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

#include <utopia2/extension.h>
#include <utopia2/library.h>
#include <utopia2/node.h>
#include <utopia2/plugin.h>
#include <utopia2/pluginmanager.h>

#include <algorithm>

namespace Utopia
{

    //
    // Class PluginManager
    //

    /**
     *  \brief Construct a default service PluginManager.
     */
    PluginManager::PluginManager()
        : InvocationSet()
    {
        // Load plugins
        this->_loadPlugins();
    }

    /**
     *  \brief Destroys a service PluginManager.
     *
     *  Unloads any loaded service plugins.
     */
    PluginManager::~PluginManager()
    {
        // Clean up invocations
        this->clear();

        // Delegate unloading of plugins
        this->_unloadPlugins();
    }

    /**
     *  \brief Returns service plugins matching input/action/output.
     *  \param input_ Service input.
     *  \param action_ Service action.
     *  \param output_ Service output.
     */
    std::set< Plugin * > PluginManager::getMatchingPlugins(std::vector< Node * > input_, Node * action_, std::vector< Node * > output_)
    {
        // Matching plugins
        std::set< Plugin * > plugins;

        // Sort input and output (shadowed!)
        sort(input_.begin(), input_.end());
        sort(output_.begin(), output_.end());

        // For each indexed service plugin see if it matches...
        std::set< Plugin * >::const_iterator iter = this->_plugins.begin();
        std::set< Plugin * >::const_iterator end = this->_plugins.end();
        for (; iter != end; ++iter)
        {
            // Does the action match?
            if (action_ != 0 && action_ != (*iter)->action())
            {
                continue;
            }

            // Does the input match?
            std::vector< Node * > input = (*iter)->input();
            sort(input.begin(), input.end());
            if (!input_.empty() && input != input_)
            {
                continue;
            }

            // Does the output match?
            std::vector< Node * > output = (*iter)->output();
            sort(output.begin(), output.end());
            if (!output_.empty() && output != output_)
            {
                continue;
            }

            // It matches!!!
            plugins.insert(*iter);
        }

        return plugins;
    }

    /**
     *  \brief Load service Plugin extensions from loaded plugins.
     */
    void PluginManager::_loadPlugins()
    {
        // Retrieve all service plugins
        this->_plugins = instantiateAllExtensions< Plugin >();
    }

    /**
     *  \brief Unload any service plugins loaded by this PluginManager.
     */
    void PluginManager::_unloadPlugins()
    {
        // Remove all plugins and unload plugins
        std::set< Plugin * >::iterator plugin_iter = this->_plugins.begin();
        std::set< Plugin * >::iterator plugin_end = this->_plugins.end();
        for (; plugin_iter != plugin_end; ++plugin_iter)
        {
            // Delete Plugin
            delete *plugin_iter;
        }
        this->_plugins.clear();
    }

} // namespace Utopia
