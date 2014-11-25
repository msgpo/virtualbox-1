/* $Id: ApplianceImpl.h 50832 2014-03-21 06:08:32Z vboxsync $ */
/** @file
 * VirtualBox COM class implementation
 */

/*
 * Copyright (C) 2006-2013 Oracle Corporation
 *
 * This file is part of VirtualBox Open Source Edition (OSE), as
 * available from http://www.virtualbox.org. This file is free software;
 * you can redistribute it and/or modify it under the terms of the GNU
 * General Public License (GPL) as published by the Free Software
 * Foundation, in version 2 as it comes in the "COPYING" file of the
 * VirtualBox OSE distribution. VirtualBox OSE is distributed in the
 * hope that it will be useful, but WITHOUT ANY WARRANTY of any kind.
 */

#ifndef ____H_APPLIANCEIMPL
#define ____H_APPLIANCEIMPL

/* VBox includes */
#include <VBox/settings.h>
#include "VirtualSystemDescriptionWrap.h"
#include "ApplianceWrap.h"
#include "MediumFormatImpl.h"

/* Todo: This file needs massive cleanup. Split IAppliance in a public and
 * private classes. */
#include <iprt/tar.h>
#include "ovfreader.h"
#include <set>

/* VBox forward declarations */
class Progress;
class VirtualSystemDescription;
struct VirtualSystemDescriptionEntry;
struct LocationInfo;
typedef struct VDINTERFACE   *PVDINTERFACE;
typedef struct VDINTERFACEIO *PVDINTERFACEIO;
typedef struct SHASTORAGE    *PSHASTORAGE;

typedef enum applianceIOName { applianceIOTar, applianceIOFile, applianceIOSha } APPLIANCEIONAME;

namespace ovf
{
    struct HardDiskController;
    struct VirtualSystem;
    class  OVFReader;
    struct DiskImage;
    struct EnvelopeData;
}

namespace xml
{
    class Document;
    class ElementNode;
}

namespace settings
{
    class MachineConfigFile;
}

class ATL_NO_VTABLE Appliance :
    public ApplianceWrap
{
public:

    DECLARE_EMPTY_CTOR_DTOR(Appliance)

    HRESULT FinalConstruct();
    void FinalRelease();


    HRESULT init(VirtualBox *aVirtualBox);
    void uninit();

    /* public methods only for internal purposes */

    static HRESULT i_setErrorStatic(HRESULT aResultCode,
                                    const Utf8Str &aText)
    {
        return setErrorInternal(aResultCode, getStaticClassIID(), getStaticComponentName(), aText, false, true);
    }

    /* private instance data */
private:
    // wrapped IAppliance properties
    HRESULT getPath(com::Utf8Str &aPath);
    HRESULT getDisks(std::vector<com::Utf8Str> &aDisks);
    HRESULT getVirtualSystemDescriptions(std::vector<ComPtr<IVirtualSystemDescription> > &aVirtualSystemDescriptions);
    HRESULT getMachines(std::vector<com::Utf8Str> &aMachines);

    // wrapped IAppliance methods
    HRESULT read(const com::Utf8Str &aFile,
                 ComPtr<IProgress> &aProgress);
    HRESULT interpret();
    HRESULT importMachines(const std::vector<ImportOptions_T> &aOptions,
                           ComPtr<IProgress> &aProgress);
    HRESULT createVFSExplorer(const com::Utf8Str &aURI,
                              ComPtr<IVFSExplorer> &aExplorer);
    HRESULT write(const com::Utf8Str &aFormat,
                  const std::vector<ExportOptions_T> &aOptions,
                  const com::Utf8Str &aPath,
                  ComPtr<IProgress> &aProgress);
    HRESULT getWarnings(std::vector<com::Utf8Str> &aWarnings);

    /** weak VirtualBox parent */
    VirtualBox* const mVirtualBox;

    struct ImportStack;
    struct TaskOVF;
    struct Data;            // opaque, defined in ApplianceImpl.cpp
    Data *m;

    enum SetUpProgressMode { ImportFile, ImportS3, WriteFile, WriteS3 };

    /** @name General stuff
     * @{
     */
    bool i_isApplianceIdle();
    HRESULT i_searchUniqueVMName(Utf8Str& aName) const;
    HRESULT i_searchUniqueDiskImageFilePath(Utf8Str& aName) const;
    HRESULT i_setUpProgress(ComObjPtr<Progress> &pProgress,
                            const Bstr &bstrDescription,
                            SetUpProgressMode mode);
    void i_waitForAsyncProgress(ComObjPtr<Progress> &pProgressThis, ComPtr<IProgress> &pProgressAsync);
    void i_addWarning(const char* aWarning, ...);
    void i_disksWeight();
    void i_parseBucket(Utf8Str &aPath, Utf8Str &aBucket);

    static DECLCALLBACK(int) i_taskThreadImportOrExport(RTTHREAD aThread, void *pvUser);

    HRESULT i_initSetOfSupportedStandardsURI();

    Utf8Str i_typeOfVirtualDiskFormatFromURI(Utf8Str type) const;

    std::set<Utf8Str> i_URIFromTypeOfVirtualDiskFormat(Utf8Str type);

    HRESULT i_initApplianceIONameMap();

    Utf8Str i_applianceIOName(APPLIANCEIONAME type) const;

    HRESULT i_findMediumFormatFromDiskImage(const ovf::DiskImage &di, ComObjPtr<MediumFormat>& mf);
    /** @}  */

    /** @name Read stuff
     * @{
     */
    HRESULT i_readImpl(const LocationInfo &aLocInfo, ComObjPtr<Progress> &aProgress);

    HRESULT i_readFS(TaskOVF *pTask);
    HRESULT i_readFSOVF(TaskOVF *pTask);
    HRESULT i_readFSOVA(TaskOVF *pTask);
    HRESULT i_readFSImpl(TaskOVF *pTask, const RTCString &strFilename, PVDINTERFACEIO pCallbacks, PSHASTORAGE pStorage);
    HRESULT i_readS3(TaskOVF *pTask);
    /** @}  */

    /** @name Import stuff
     * @}
     */
    HRESULT i_importImpl(const LocationInfo &aLocInfo, ComObjPtr<Progress> &aProgress);

    HRESULT i_importFS(TaskOVF *pTask);
    HRESULT i_importFSOVF(TaskOVF *pTask, AutoWriteLockBase& writeLock);
    HRESULT i_importFSOVA(TaskOVF *pTask, AutoWriteLockBase& writeLock);
    HRESULT i_importS3(TaskOVF *pTask);

    HRESULT i_readFileToBuf(const Utf8Str &strFile,
                            void **ppvBuf,
                            size_t *pcbSize,
                            bool fCreateDigest,
                            PVDINTERFACEIO pCallbacks,
                            PSHASTORAGE pStorage);
    HRESULT i_readTarFileToBuf(struct FSSRDONLYINTERFACEIO *pTarIo,
                               const Utf8Str &strFile,
                               void **ppvBuf,
                               size_t *pcbSize,
                               bool fCreateDigest,
                               PVDINTERFACEIO pCallbacks,
                               PSHASTORAGE pStorage);
    HRESULT i_verifyManifestFile(const Utf8Str &strFile, ImportStack &stack, void *pvBuf, size_t cbSize);

    HRESULT i_verifyCertificateFile(void *pvBuf, size_t cbSize, PSHASTORAGE pStorage);

    void i_convertDiskAttachmentValues(const ovf::HardDiskController &hdc,
                                       uint32_t ulAddressOnParent,
                                       Bstr &controllerType,
                                       int32_t &lControllerPort,
                                       int32_t &lDevice);

    void i_importOneDiskImage(const ovf::DiskImage &di,
                              Utf8Str *strTargetPath,
                              ComObjPtr<Medium> &pTargetHD,
                              ImportStack &stack,
                              PVDINTERFACEIO pCallbacks,
                              PSHASTORAGE pStorage);

    void i_importMachineGeneric(const ovf::VirtualSystem &vsysThis,
                                ComObjPtr<VirtualSystemDescription> &vsdescThis,
                                ComPtr<IMachine> &pNewMachine,
                                ImportStack &stack,
                                PVDINTERFACEIO pCallbacks,
                                PSHASTORAGE pStorage);
    void i_importVBoxMachine(ComObjPtr<VirtualSystemDescription> &vsdescThis,
                             ComPtr<IMachine> &pNewMachine,
                             ImportStack &stack,
                             PVDINTERFACEIO pCallbacks,
                             PSHASTORAGE pStorage);
    void i_importMachines(ImportStack &stack,
                          PVDINTERFACEIO pCallbacks,
                          PSHASTORAGE pStorage);

    HRESULT i_preCheckImageAvailability(PSHASTORAGE pSHAStorage,
                                        RTCString &availableImage);
    /** @} */

    /** @name Write stuff
     * @{
     */
    HRESULT i_writeImpl(ovf::OVFVersion_T aFormat, const LocationInfo &aLocInfo, ComObjPtr<Progress> &aProgress);

    HRESULT i_writeFS(TaskOVF *pTask);
    HRESULT i_writeFSOVF(TaskOVF *pTask, AutoWriteLockBase& writeLock);
    HRESULT i_writeFSOVA(TaskOVF *pTask, AutoWriteLockBase& writeLock);
    HRESULT i_writeFSImpl(TaskOVF *pTask, AutoWriteLockBase& writeLock, PVDINTERFACEIO pCallbacks, PSHASTORAGE pStorage);
    HRESULT i_writeS3(TaskOVF *pTask);

    struct XMLStack;

    void i_buildXML(AutoWriteLockBase& writeLock,
                    xml::Document &doc,
                    XMLStack &stack,
                    const Utf8Str &strPath,
                    ovf::OVFVersion_T enFormat);
    void i_buildXMLForOneVirtualSystem(AutoWriteLockBase& writeLock,
                                       xml::ElementNode &elmToAddVirtualSystemsTo,
                                       std::list<xml::ElementNode*> *pllElementsWithUuidAttributes,
                                       ComObjPtr<VirtualSystemDescription> &vsdescThis,
                                       ovf::OVFVersion_T enFormat,
                                       XMLStack &stack);
    /** @} */

    friend class Machine;
};

void i_parseURI(Utf8Str strUri, LocationInfo &locInfo);

struct VirtualSystemDescriptionEntry
{
    uint32_t ulIndex;                       ///< zero-based index of this entry within array
    VirtualSystemDescriptionType_T type;    ///< type of this entry
    Utf8Str strRef;                         ///< reference number (hard disk controllers only)
    Utf8Str strOvf;                         ///< original OVF value (type-dependent)
    Utf8Str strVBoxSuggested;               ///< configuration value (type-dependent); original value suggested by interpret()
    Utf8Str strVBoxCurrent;                 ///< configuration value (type-dependent); current value, either from interpret() or setFinalValue()
    Utf8Str strExtraConfigSuggested;        ///< extra configuration key=value strings (type-dependent); original value suggested by interpret()
    Utf8Str strExtraConfigCurrent;          ///< extra configuration key=value strings (type-dependent); current value, either from interpret() or setFinalValue()

    uint32_t ulSizeMB;                      ///< hard disk images only: a copy of ovf::DiskImage::ulSuggestedSizeMB
    bool skipIt;                            ///< used during export to skip some parts if it's needed
};

class ATL_NO_VTABLE VirtualSystemDescription :
    public VirtualSystemDescriptionWrap
{
    friend class Appliance;

public:

    DECLARE_EMPTY_CTOR_DTOR(VirtualSystemDescription)

    HRESULT FinalConstruct();
    void FinalRelease();

    HRESULT init();
    void uninit();

    /* public methods only for internal purposes */
    void i_addEntry(VirtualSystemDescriptionType_T aType,
                    const Utf8Str &strRef,
                    const Utf8Str &aOvfValue,
                    const Utf8Str &aVBoxValue,
                    uint32_t ulSizeMB = 0,
                    const Utf8Str &strExtraConfig = "");

    std::list<VirtualSystemDescriptionEntry*> i_findByType(VirtualSystemDescriptionType_T aType);
    const VirtualSystemDescriptionEntry* i_findControllerFromID(uint32_t id);

    void i_importVBoxMachineXML(const xml::ElementNode &elmMachine);
    const settings::MachineConfigFile* i_getMachineConfig() const;

    /* private instance data */
private:

    // wrapped IVirtualSystemDescription properties
    HRESULT getCount(ULONG *aCount);

    // wrapped IVirtualSystemDescription methods
    HRESULT getDescription(std::vector<VirtualSystemDescriptionType_T> &aTypes,
                           std::vector<com::Utf8Str> &aRefs,
                           std::vector<com::Utf8Str> &aOVFValues,
                           std::vector<com::Utf8Str> &aVBoxValues,
                           std::vector<com::Utf8Str> &aExtraConfigValues);
    HRESULT getDescriptionByType(VirtualSystemDescriptionType_T aType,
                                 std::vector<VirtualSystemDescriptionType_T> &aTypes,
                                 std::vector<com::Utf8Str> &aRefs,
                                 std::vector<com::Utf8Str> &aOVFValues,
                                 std::vector<com::Utf8Str> &aVBoxValues,
                                 std::vector<com::Utf8Str> &aExtraConfigValues);
    HRESULT getValuesByType(VirtualSystemDescriptionType_T aType,
                            VirtualSystemDescriptionValueType_T aWhich,
                            std::vector<com::Utf8Str> &aValues);
    HRESULT setFinalValues(const std::vector<BOOL> &aEnabled,
                           const std::vector<com::Utf8Str> &aVBoxValues,
                           const std::vector<com::Utf8Str> &aExtraConfigValues);
    HRESULT addDescription(VirtualSystemDescriptionType_T aType,
                           const com::Utf8Str &aVBoxValue,
                           const com::Utf8Str &aExtraConfigValue);
    void i_removeByType(VirtualSystemDescriptionType_T aType);

    struct Data;
    Data *m;

    friend class Machine;
};

#endif // !____H_APPLIANCEIMPL
/* vi: set tabstop=4 shiftwidth=4 expandtab: */