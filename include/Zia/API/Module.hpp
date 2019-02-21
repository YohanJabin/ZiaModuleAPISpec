/*
** EPITECH PROJECT, 2019
** ZiaModuleAPISpec
** File description:
** Module spec
*/

#pragma once

#include <string>
#include <memory>

#include "ServerConfig.hpp"
#include "RequestHandler.hpp"

namespace Zia {
namespace API {

/*
 * BASE MODULE
 *
 * This class is the main module interface. It describes the module meta-data,
 * and contains server-level hooks for this module.
 *
 * Use exceptions for critial errors in the module code. Those can be catched
 * by the server in order to shut down the module and prevent a crash.
 */
class Module {
public:
    using pointer = std::shared_ptr<Module>;

    /*
     * INSTANTIATION:
     *
     * constructor:
     *     used to allocated needed resources for said module
     * destructor:
     *     used to deallocate those same resources
     */
    virtual ~Module() {};

    /*
     * METADATA:
     *
     * getName:
     *     name of the module
     */
    virtual const std::string& getName() = 0;

    /*
     * ACTIVATION HOOKS:
     *
     * onActivate:
     *     called when the module is activated by the server
     *     server configuration is passed as an argument
     * onDeactivate:
     *     called when the module is deactivated
     * onConfigChange:
     *     called when the server configuration is updated
     */
    virtual void onActivate(const ServerConfig& cfg) = 0;
    virtual void onDeactivate() = 0;
    virtual void onConfigChange(const ServerConfig& cfg) = 0;

    /*
     * REQUEST HANDLER:
     *
     * newRequestHandler:
     *     creates a new instance of a Request Handler.
     */
    virtual RequestHandler::pointer newRequestHandler() = 0;

    /*
     * SERVER-LEVEL HOOKS:
     *
     * [TBD]
     */
};

}
}
