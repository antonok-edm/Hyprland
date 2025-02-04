#pragma once

#include <memory>
#include <deque>
#include <list>
#include <sys/resource.h>

#include "defines.hpp"
#include "debug/Log.hpp"
#include "events/Events.hpp"
#include "config/ConfigManager.hpp"
#include "managers/ThreadManager.hpp"
#include "managers/XWaylandManager.hpp"
#include "managers/input/InputManager.hpp"
#include "managers/LayoutManager.hpp"
#include "managers/KeybindManager.hpp"
#include "managers/AnimationManager.hpp"
#include "managers/EventManager.hpp"
#include "managers/ProtocolManager.hpp"
#include "managers/SessionLockManager.hpp"
#include "managers/HookSystemManager.hpp"
#include "debug/HyprDebugOverlay.hpp"
#include "debug/HyprNotificationOverlay.hpp"
#include "helpers/Monitor.hpp"
#include "desktop/Workspace.hpp"
#include "desktop/Window.hpp"
#include "render/Renderer.hpp"
#include "render/OpenGL.hpp"
#include "hyprerror/HyprError.hpp"
#include "plugins/PluginSystem.hpp"
#include "helpers/Watchdog.hpp"

#include <aquamarine/backend/Backend.hpp>
#include <aquamarine/output/Output.hpp>

class CWLSurfaceResource;

enum eManagersInitStage {
    STAGE_PRIORITY = 0,
    STAGE_BASICINIT,
    STAGE_LATE
};

class CCompositor {
  public:
    CCompositor();
    ~CCompositor();

    wl_display*                               m_sWLDisplay;
    wl_event_loop*                            m_sWLEventLoop;
    int                                       m_iDRMFD       = -1;
    bool                                      m_bInitialized = false;
    SP<Aquamarine::CBackend>                  m_pAqBackend;

    std::string                               m_szHyprTempDataRoot = "";

    std::string                               m_szWLDisplaySocket   = "";
    std::string                               m_szInstanceSignature = "";
    std::string                               m_szInstancePath      = "";
    std::string                               m_szCurrentSplash     = "error";

    std::vector<SP<CMonitor>>                 m_vMonitors;
    std::vector<SP<CMonitor>>                 m_vRealMonitors; // for all monitors, even those turned off
    std::vector<PHLWINDOW>                    m_vWindows;
    std::vector<PHLLS>                        m_vLayers;
    std::vector<PHLWORKSPACE>                 m_vWorkspaces;
    std::vector<PHLWINDOWREF>                 m_vWindowsFadingOut;
    std::vector<PHLLSREF>                     m_vSurfacesFadingOut;

    std::unordered_map<std::string, uint64_t> m_mMonitorIDMap;

    void                                      initServer(std::string socketName, int socketFd);
    void                                      startCompositor();
    void                                      cleanup();
    void                                      createLockFile();
    void                                      removeLockFile();
    void                                      bumpNofile();
    void                                      restoreNofile();

    WP<CWLSurfaceResource>                    m_pLastFocus;
    PHLWINDOWREF                              m_pLastWindow;
    WP<CMonitor>                              m_pLastMonitor;

    std::vector<PHLWINDOWREF>                 m_vWindowFocusHistory; // first element is the most recently focused.

    bool                                      m_bReadyToProcess = false;
    bool                                      m_bSessionActive  = true;
    bool                                      m_bDPMSStateON    = true;
    bool                                      m_bUnsafeState    = false; // unsafe state is when there is no monitors.
    bool                                      m_bNextIsUnsafe   = false;
    CMonitor*                                 m_pUnsafeOutput   = nullptr; // fallback output for the unsafe state
    bool                                      m_bIsShuttingDown = false;

    // ------------------------------------------------- //

    CMonitor*              getMonitorFromID(const int&);
    CMonitor*              getMonitorFromName(const std::string&);
    CMonitor*              getMonitorFromDesc(const std::string&);
    CMonitor*              getMonitorFromCursor();
    CMonitor*              getMonitorFromVector(const Vector2D&);
    void                   removeWindowFromVectorSafe(PHLWINDOW);
    void                   focusWindow(PHLWINDOW, SP<CWLSurfaceResource> pSurface = nullptr);
    void                   focusSurface(SP<CWLSurfaceResource>, PHLWINDOW pWindowOwner = nullptr);
    bool                   monitorExists(CMonitor*);
    PHLWINDOW              vectorToWindowUnified(const Vector2D&, uint8_t properties, PHLWINDOW pIgnoreWindow = nullptr);
    SP<CWLSurfaceResource> vectorToLayerSurface(const Vector2D&, std::vector<PHLLSREF>*, Vector2D*, PHLLS*);
    SP<CWLSurfaceResource> vectorToLayerPopupSurface(const Vector2D&, CMonitor* monitor, Vector2D*, PHLLS*);
    SP<CWLSurfaceResource> vectorWindowToSurface(const Vector2D&, PHLWINDOW, Vector2D& sl);
    Vector2D               vectorToSurfaceLocal(const Vector2D&, PHLWINDOW, SP<CWLSurfaceResource>);
    CMonitor*              getMonitorFromOutput(SP<Aquamarine::IOutput>);
    CMonitor*              getRealMonitorFromOutput(SP<Aquamarine::IOutput>);
    PHLWINDOW              getWindowFromSurface(SP<CWLSurfaceResource>);
    PHLWINDOW              getWindowFromHandle(uint32_t);
    bool                   isWorkspaceVisible(PHLWORKSPACE);
    bool                   isWorkspaceVisibleNotCovered(PHLWORKSPACE);
    PHLWORKSPACE           getWorkspaceByID(const int&);
    PHLWORKSPACE           getWorkspaceByName(const std::string&);
    PHLWORKSPACE           getWorkspaceByString(const std::string&);
    void                   sanityCheckWorkspaces();
    void                   updateWorkspaceWindowDecos(const int&);
    void                   updateWorkspaceWindowData(const int&);
    int                    getWindowsOnWorkspace(const int& id, std::optional<bool> onlyTiled = {}, std::optional<bool> onlyVisible = {});
    int                    getGroupsOnWorkspace(const int& id, std::optional<bool> onlyTiled = {}, std::optional<bool> onlyVisible = {});
    PHLWINDOW              getUrgentWindow();
    bool                   hasUrgentWindowOnWorkspace(const int&);
    PHLWINDOW              getFirstWindowOnWorkspace(const int&);
    PHLWINDOW              getTopLeftWindowOnWorkspace(const int&);
    PHLWINDOW              getFullscreenWindowOnWorkspace(const int&);
    bool                   isWindowActive(PHLWINDOW);
    void                   changeWindowZOrder(PHLWINDOW, bool);
    void                   cleanupFadingOut(const int& monid);
    PHLWINDOW              getWindowInDirection(PHLWINDOW, char);
    PHLWINDOW              getNextWindowOnWorkspace(PHLWINDOW, bool focusableOnly = false, std::optional<bool> floating = {});
    PHLWINDOW              getPrevWindowOnWorkspace(PHLWINDOW, bool focusableOnly = false, std::optional<bool> floating = {});
    int                    getNextAvailableNamedWorkspace();
    bool                   isPointOnAnyMonitor(const Vector2D&);
    bool                   isPointOnReservedArea(const Vector2D& point, const CMonitor* monitor = nullptr);
    CMonitor*              getMonitorInDirection(const char&);
    CMonitor*              getMonitorInDirection(CMonitor*, const char&);
    void                   updateAllWindowsAnimatedDecorationValues();
    void                   updateWorkspaceWindows(const int64_t& id);
    void                   updateWindowAnimatedDecorationValues(PHLWINDOW);
    int                    getNextAvailableMonitorID(std::string const& name);
    void                   moveWorkspaceToMonitor(PHLWORKSPACE, CMonitor*, bool noWarpCursor = false);
    void                   swapActiveWorkspaces(CMonitor*, CMonitor*);
    CMonitor*              getMonitorFromString(const std::string&);
    bool                   workspaceIDOutOfBounds(const int64_t&);
    void                   setWindowFullscreen(PHLWINDOW, bool, eFullscreenMode mode = FULLSCREEN_INVALID);
    void                   updateFullscreenFadeOnWorkspace(PHLWORKSPACE);
    PHLWINDOW              getX11Parent(PHLWINDOW);
    void                   scheduleFrameForMonitor(CMonitor*, Aquamarine::IOutput::scheduleFrameReason reason = Aquamarine::IOutput::AQ_SCHEDULE_CLIENT_UNKNOWN);
    void                   addToFadingOutSafe(PHLLS);
    void                   removeFromFadingOutSafe(PHLLS);
    void                   addToFadingOutSafe(PHLWINDOW);
    PHLWINDOW              getWindowByRegex(const std::string&);
    void                   warpCursorTo(const Vector2D&, bool force = false);
    PHLLS                  getLayerSurfaceFromSurface(SP<CWLSurfaceResource>);
    void                   closeWindow(PHLWINDOW);
    Vector2D               parseWindowVectorArgsRelative(const std::string&, const Vector2D&);
    void                   forceReportSizesToWindowsOnWorkspace(const int&);
    PHLWORKSPACE           createNewWorkspace(const int&, const int&, const std::string& name = "", bool isEmtpy = true); // will be deleted next frame if left empty and unfocused!
    void                   renameWorkspace(const int&, const std::string& name = "");
    void                   setActiveMonitor(CMonitor*);
    bool                   isWorkspaceSpecial(const int&);
    int                    getNewSpecialID();
    void                   performUserChecks();
    void                   moveWindowToWorkspaceSafe(PHLWINDOW pWindow, PHLWORKSPACE pWorkspace);
    PHLWINDOW              getForceFocus();
    void                   arrangeMonitors();
    void                   enterUnsafeState();
    void                   leaveUnsafeState();
    void                   setPreferredScaleForSurface(SP<CWLSurfaceResource> pSurface, double scale);
    void                   setPreferredTransformForSurface(SP<CWLSurfaceResource> pSurface, wl_output_transform transform);
    void                   updateSuspendedStates();
    PHLWINDOW              windowForCPointer(CWindow*);
    void                   onNewMonitor(SP<Aquamarine::IOutput> output);

    std::string            explicitConfigPath;

  private:
    void             initAllSignals();
    void             removeAllSignals();
    void             cleanEnvironment();
    void             setRandomSplash();
    void             initManagers(eManagersInitStage stage);
    void             prepareFallbackOutput();

    uint64_t         m_iHyprlandPID    = 0;
    wl_event_source* m_critSigSource   = nullptr;
    rlimit           m_sOriginalNofile = {0};
};

inline std::unique_ptr<CCompositor> g_pCompositor;
