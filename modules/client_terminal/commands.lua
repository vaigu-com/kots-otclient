function pcolored(text, color)
    color = color or 'white'
    modules.client_terminal.addLine(tostring(text), color)
end

function draw_debug_boxes()
    g_ui.setDebugBoxesDrawing(not g_ui.isDrawingDebugBoxes())
end

function hide_map()
    modules.game_interface.getMapPanel():hide()
end

function show_map()
    modules.game_interface.getMapPanel():show()
end

function live_textures_reload()
    g_textures.liveReload()
end

local pinging = false
local function pingBack(ping)
    if ping < 300 then
        color = 'green'
    elseif ping < 600 then
        color = 'yellow'
    else
        color = 'red'
    end
    pcolored(g_game.getWorldName() .. ' => ' .. ping .. ' ms', color)
end
function ping()
    if pinging then
        pcolored('Ping stopped.')
        g_game.setPingDelay(1000)
        disconnect(g_game, 'onPingBack', pingBack)
    else
        if not (g_game.getFeature(GameClientPing) or g_game.getFeature(GameExtendedClientPing)) then
            pcolored('this server does not support ping', 'red')
            return
        elseif not g_game.isOnline() then
            pcolored('ping command is only allowed when online', 'red')
            return
        end

        pcolored('Starting ping...')
        g_game.setPingDelay(0)
        connect(g_game, 'onPingBack', pingBack)
    end
    pinging = not pinging
end

function clear()
    modules.client_terminal.clear()
end

function ls(path)
    path = path or '/'
    local files = g_resources.listDirectoryFiles(path)
    for k, v in pairs(files) do
        if g_resources.directoryExists(path .. v) then
            pcolored(path .. v, 'blue')
        else
            pcolored(path .. v)
        end
    end
end

function about_version()
    pcolored(g_app.getName() .. ' ' .. g_app.getVersion() .. '\n' .. 'Rev  ' .. g_app.getBuildRevision() .. ' (' ..
                 g_app.getBuildCommit() .. ')\n' .. 'Built on ' .. g_app.getBuildDate())
end

function about_graphics()
    pcolored('Vendor ' .. g_graphics.getVendor())
    pcolored('Renderer' .. g_graphics.getRenderer())
    pcolored('Version' .. g_graphics.getVersion())
end

function about_modules()
    for k, m in pairs(g_modules.getModules()) do
        local loadedtext
        if m:isLoaded() then
            pcolored(m:getName() .. ' => loaded', 'green')
        else
            pcolored(m:getName() .. ' => not loaded', 'red')
        end
    end
end

-- Widget inspector: hover any UI element to see its id, class, geometry and
-- parent chain in a small box next to the cursor. Toggle from the terminal:
--   inspect_widgets()
local widgetInspector = { overlay = nil, event = nil }

local function widgetInspectorLabel(w)
    local id = w:getId()
    if id == nil or id == '' then id = '<no-id>' end
    return id .. '  (' .. w:getClassName() .. ')'
end

function inspect_widgets()
    if widgetInspector.event then
        widgetInspector.event:cancel()
        widgetInspector.event = nil
        if widgetInspector.overlay then
            widgetInspector.overlay:destroy()
            widgetInspector.overlay = nil
        end
        pcolored('Widget inspector: OFF')
        return
    end

    local overlay = g_ui.createWidget('UILabel', rootWidget)
    overlay:setId('widgetInspectorOverlay')
    overlay:mergeStyle({
        ['background-color'] = '#000000dd',
        ['border-width'] = 1,
        ['border-color'] = '#00ff00',
        ['color'] = '#00ff00',
        ['font'] = 'verdana-11px-monochrome',
        ['text-align'] = 'left',
        ['text-auto-resize'] = true,
        ['padding'] = 3,
        ['phantom'] = true
    })
    widgetInspector.overlay = overlay

    widgetInspector.event = cycleEvent(function()
        local pos = g_window.getMousePosition()
        local w = rootWidget:recursiveGetChildByPos(pos, true)
        if not w or w == overlay then
            overlay:hide()
            return
        end

        local lines = { widgetInspectorLabel(w) }
        local rect = w:getRect()
        lines[#lines + 1] = string.format('rect: %d,%d  %dx%d', rect.x, rect.y, rect.width, rect.height)

        local cur = w:getParent()
        local depth = 0
        while cur and cur ~= rootWidget and depth < 4 do
            lines[#lines + 1] = '^ ' .. widgetInspectorLabel(cur)
            cur = cur:getParent()
            depth = depth + 1
        end

        overlay:setText(table.concat(lines, '\n'))
        overlay:show()
        overlay:raise()

        local sz = overlay:getSize()
        local scr = rootWidget:getSize()
        local x = pos.x + 14
        local y = pos.y + 14
        if x + sz.width > scr.width then x = pos.x - sz.width - 6 end
        if y + sz.height > scr.height then y = pos.y - sz.height - 6 end
        overlay:setPosition({ x = math.max(0, x), y = math.max(0, y) })
    end, 50)

    pcolored('Widget inspector: ON — hover elements; run inspect_widgets() again to stop.', 'green')
end
