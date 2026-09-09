local iconTopMenu = nil
local function healthManaEvent()
    local player = g_game.getLocalPlayer()
    if not player then
        return
    end

    healthManaController.ui.health.text:setText(player:getHealth())
    healthManaController.ui.health.current:setWidth(math.max(12, math.ceil(
        (healthManaController.ui.health.total:getWidth() * player:getHealth()) / player:getMaxHealth())))

    healthManaController.ui.mana.text:setText(player:getMana())
    healthManaController.ui.mana.current:setWidth(math.max(12, math.ceil(
        (healthManaController.ui.mana.total:getWidth() * player:getMana()) / player:getMaxMana())))
end

healthManaController = Controller:new()
healthManaController:setUI('healthinfo', modules.game_interface.getMainRightPanel())

function healthManaController:onInit()
    -- The dedicated health/mana pane on the right panel has been removed;
    -- health and mana are shown in the top stats bar instead. Detach and
    -- destroy the widget so it takes no space in the right panel.
    if healthManaController.ui then
        local parent = healthManaController.ui:getParent()
        if parent then
            parent:removeChild(healthManaController.ui)
        end
        healthManaController.ui:destroy()
        healthManaController.ui = nil
    end
end

function healthManaController:onTerminate()
    if iconTopMenu then
        iconTopMenu:destroy()
        iconTopMenu = nil
    end
end

function healthManaController:onGameStart()
    -- Right-panel health/mana pane removed; nothing to update here.
end

function extendedView(extendedView)
    -- The health/mana pane has been removed from the right panel, so there is
    -- nothing to toggle here. Kept as a no-op so game_interface's extended-view
    -- switch still has a valid entry point.
end

function toggle()
end
