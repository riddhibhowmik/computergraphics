#pragma once

class LightningFlashReactive{
    public:
        virtual ~LightningFlashReactive() = default;
        virtual void SetLightningFlashAlpha(float flashAlpha) = 0;
};