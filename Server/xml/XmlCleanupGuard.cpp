#include "XmlCleanupGuard.h"
#include <libxml/parser.h>

XmlCleanupGuard::~XmlCleanupGuard() noexcept
{
    xmlCleanupParser();
}
