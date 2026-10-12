/*
 * mod-vendor-item-preload loader.
 *
 * AzerothCore looks up a loader symbol derived from the module's folder name: for folder
 * "mod-vendor-item-preload" that symbol is exactly "Addmod_vendor_item_preloadScripts". If you
 * clone the repo under a different folder name, rename this function to match.
 *
 * Released under the MIT License.
 */

void AddVendorItemPreloadScripts();

void Addmod_vendor_item_preloadScripts()
{
    AddVendorItemPreloadScripts();
}
