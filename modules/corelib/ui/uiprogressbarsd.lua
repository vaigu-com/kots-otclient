-- @docclass
UIProgressBarSD = extends(UIWidget, "UIProgressBarSD")

function UIProgressBarSD.create()
  local progressbar = UIProgressBarSD.internalCreate()
  progressbar:setFocusable(false)
  progressbar:setOn(true)
  progressbar.minimum = 0
  progressbar.maximum = 100
  progressbar.value = 0
  progressbar.bgBorderLeft = 0
  progressbar.bgBorderRight = 0
  progressbar.bgBorderTop = 0
  progressbar.bgBorderBottom = 0
  return progressbar
end

function UIProgressBarSD:setMinimum(minimum)
  self.minimum = minimum
  if self.value < minimum then
    self:setValue(minimum)
  end
end

function UIProgressBarSD:setMaximum(maximum)
  self.maximum = maximum
  if self.value > maximum then
    self:setValue(maximum)
  end
end

function UIProgressBarSD:setValue(value, minimum, maximum)
  if minimum then
    self:setMinimum(minimum)
  end

  if maximum then
    self:setMaximum(maximum)
  end

  self.value = math.max(math.min(value, self.maximum), self.minimum)
  self:updateBackground()
end

function UIProgressBarSD:setPercent(percent)
  self:setValue(percent, 0, 100)
end

function UIProgressBarSD:getPercent()
  return self.value
end

function UIProgressBarSD:getPercentPixels()
  return (self.maximum - self.minimum) / self:getWidth()
end

function UIProgressBarSD:getProgress()
  if self.minimum == self.maximum then return 1 end
  return (self.value - self.minimum) / (self.maximum - self.minimum)
end

function UIProgressBarSD:updateBackground()
  if self:isOn() then
    -- Remember the fill colour once so we can restore it (some bars set a custom image-color).
    if self.baseImageColor == nil then
      self.baseImageColor = self:getImageColor()
    end

    local fill = self:getProgress() * (self:getWidth() - self.bgBorderLeft - self.bgBorderRight)
    -- A zero-width imageRect is treated as invalid by the renderer, which then draws the FULL image (so an empty
    -- bar looked full). Keep a valid 1px rect but hide the fill by making it transparent when there is nothing to
    -- show; a positive fill is clamped up to 1px so a tiny non-zero value stays visible.
    if fill <= 0 then
      self:setImageColor('alpha')
    else
      self:setImageColor(self.baseImageColor)
    end

    local width = math.round(math.max(fill, 1))
    local height = self:getHeight() - self.bgBorderTop - self.bgBorderBottom
    local rect = { x = self.bgBorderLeft, y = self.bgBorderTop, width = width, height = height }

    self:setImageRect(rect)
  end
end

function UIProgressBarSD:onSetup()
  self:updateBackground()
end

function UIProgressBarSD:onStyleApply(name, node)
  for name,value in pairs(node) do
    if name == 'background-border-left' then
      self.bgBorderLeft = tonumber(value)
    elseif name == 'background-border-right' then
      self.bgBorderRight = tonumber(value)
    elseif name == 'background-border-top' then
      self.bgBorderTop = tonumber(value)
    elseif name == 'background-border-bottom' then
      self.bgBorderBottom = tonumber(value)
    elseif name == 'background-border' then
      self.bgBorderLeft = tonumber(value)
      self.bgBorderRight = tonumber(value)
      self.bgBorderTop = tonumber(value)
      self.bgBorderBottom = tonumber(value)
    elseif name == 'percent' then
      self.percent = self:setPercent(tonumber(value))
    elseif name == 'tooltip-delayed' then
      self.tooltipDelayed = value
    end
  end
end

function UIProgressBarSD:onGeometryChange(oldRect, newRect)
  if not self:isOn() then
    self:setHeight(0)
  end
  self:updateBackground()
end
