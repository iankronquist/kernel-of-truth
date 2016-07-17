#ifndef MOD_H
#define MOD_H

struct mod_version {
        const char *name;
        // Modules are versioned according to semver.
        int major_version;
        int minor_version;
        int patch_version;
};

struct module {
    struct mod_version version;
    // Modules have authors and licenses.
    const char *author;
    const char *license;
    // Modules may depend on a certain version of the kernel
    int kernel_major;
    int kernel_minor;
    int kernel_patch;

    // Modules can require that other modules be loaded first.
    unsigned int num_deps;
    struct mod_version *depends;

    // Modules have setup and teardown methods which will be called when they
    // are loaded or unloaded.
    int (*setup)(void);
    int (*teardown)(void);
};

#define MODULE(name, author, license) \
    struct module name = { \
        .version.name = #name, \
        .author = #author, \
        .license = #license, \
        .version.major_version = 0, \
        .version.minor_version = 0, \
        .version.patch_version = 0, \
        .setup = NULL, \
        .teardown = NULL, \
    }; \
    static const struct module *this_module = &name; \

#define MOD_SETUP(setup) \
    this_module->setup = setup;

#define MOD_TEARDOWN(setup) \
    this_module->teardown = teardown;

#ifdef __GNUC__
#define MOD_DEPENDS(dependencies...) \
    struct mod_version this_depends [] = { dependencies, {0} }; \
    this_module->depends = &this_depends; \
    this_module->num_deps = sizeof(this_depends);
#endif


#endif
