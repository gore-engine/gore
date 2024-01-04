#include "Prefix.h"

#include <string>

#import <Foundation/Foundation.h>

namespace gore
{

std::string GetMacOSAppBundleResourcePath()
{
    NSString* resourcePath = [[NSBundle mainBundle] resourcePath];
    return [resourcePath UTF8String];
}

}