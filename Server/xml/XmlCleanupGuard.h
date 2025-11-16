#pragma once

class XmlCleanupGuard {
public:
    XmlCleanupGuard() = default;
    ~XmlCleanupGuard() noexcept;
};
