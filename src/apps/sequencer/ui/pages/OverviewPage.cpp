#include "OverviewPage.h"

#include "model/NoteTrack.h"

#include "ui/painters/WindowPainter.h"
#include "ui/LedPainter.h"
#include "ui/painters/SequencePainter.h"

static void drawNoteTrack(Canvas &canvas, int trackIndex, const NoteTrackEngine &trackEngine, NoteSequence &sequence, bool running, bool patternFollow) {
    canvas.setBlendMode(BlendMode::Set);

    int stepOffset = 16*sequence.section();
    if (patternFollow) {
        int section_no = int((trackEngine.currentStep()) / 16);
        sequence.setSecion(section_no);
    }
    int y = trackIndex * 8;

    for (int i = 0; i < 16; ++i) {
        int stepIndex = stepOffset + i;
        const auto &step = sequence.step(stepIndex);

        int x = 68 + i * 8;

        if (trackEngine.currentStep() == stepIndex) {
            canvas.setColor(step.gate() ? Color::Bright : Color::MediumBright);
            canvas.fillRect(x + 1, y + 1, 6, 6);
        } else {
            canvas.setColor(step.gate() ? Color::Medium : Color::Low);
            canvas.fillRect(x + 1, y + 1, 6, 6);
        }

        // if (trackEngine.currentStep() == stepIndex) {
        //     canvas.setColor(Color::Bright);
        //     canvas.drawRect(x + 1, y + 1, 6, 6);
        // }
    }
}

static void drawLogicTrack(Canvas &canvas, int trackIndex, const LogicTrackEngine &trackEngine, LogicSequence &sequence, bool running, bool patternFollow) {
    canvas.setBlendMode(BlendMode::Set);

    int stepOffset = 16*sequence.section();
    if (patternFollow) {
        int section_no = int((trackEngine.currentStep()) / 16);
        sequence.setSecion(section_no);
    }
    int y = trackIndex * 8;

    for (int i = 0; i < 16; ++i) {
        int stepIndex = stepOffset + i;
        const auto &step = sequence.step(stepIndex);

        int x = 68 + i * 8;

        if (trackEngine.currentStep() == stepIndex) {
            canvas.setColor(step.gate() ? Color::Bright : Color::MediumBright);

            if (trackEngine.gateOutput(stepIndex)) {
                canvas.fillRect(x + 3, y + 3, 3, 3);
                canvas.setColor(Color::Medium);
            } else {
                canvas.fillRect(x + 1, y + 1, 6, 6);
            }
        } else {
            canvas.setColor(step.gate() ? Color::Medium : Color::Low);
            canvas.fillRect(x + 1, y + 1, 6, 6);
        }
    }
}

static void drawCurve(Canvas &canvas, int x, int y, int w, int h, float &lastY, const Curve::Function function, float min, float max) {
    const int Step = 1;

    auto eval = [=] (float x) {
        return (1.f - (function(x) * (max - min) + min)) * h;
    };

    float fy0 = y + eval(0.f);

    if (lastY >= 0.f && lastY != fy0) {
        canvas.line(x, lastY, x, fy0);
    }

    for (int i = 0; i < w; i += Step) {
        float fy1 = y + eval((float(i) + Step) / w);
        canvas.line(x + i, fy0, x + i + Step, fy1);
        fy0 = fy1;
    }

    lastY = fy0;
}

static void drawStochasticTrack(Canvas &canvas, int trackIndex, const StochasticEngine &trackEngine, const StochasticSequence &sequence) {
    canvas.setBlendMode(BlendMode::Set);

    int stepOffset = (std::max(0, trackEngine.currentStep()) / 12) * 12;
    int y = trackIndex * 8;

    for (int i = 0; i < 12; ++i) {
        int stepIndex = stepOffset + i;
        const auto &step = sequence.step(stepIndex);

        int x = 16 + (68 + i * 8);

        if (trackEngine.currentStep() == stepIndex) {
            canvas.setColor(step.gate() ? Color::Bright : Color::MediumBright);
            canvas.fillRect(x + 1, y + 1, 6, 6);
        } else {
            canvas.setColor(step.gate() ? Color::Medium : Color::Low);
            canvas.fillRect(x + 1, y + 1, 6, 6);
        }
    }

}

static void drawCurveTrack(Canvas &canvas, int trackIndex, const CurveTrackEngine &trackEngine, CurveSequence &sequence, bool running, bool patternFollow) {
    canvas.setBlendMode(BlendMode::Add);
    canvas.setColor(Color::MediumBright);

    int stepOffset = 16*sequence.section();
    if (patternFollow) {
        int section_no = int((trackEngine.currentStep()) / 16);
        sequence.setSecion(section_no);
    }
    int y = trackIndex * 8;

    float lastY = -1.f;

    for (int i = 0; i < 16; ++i) {
        int stepIndex = stepOffset + i;
        const auto &step = sequence.step(stepIndex);
        float min = step.minNormalized();
        float max = step.maxNormalized();
        const auto function = Curve::function(Curve::Type(std::min(Curve::Last - 1, step.shape())));

        int x = 68 + i * 8;

        drawCurve(canvas, x, y + 1, 8, 6, lastY, function, min, max);
    }

    if (trackEngine.currentStep() >= 0) {
        int x = 64 + ((trackEngine.currentStep() - stepOffset) + trackEngine.currentStepFraction()) * 8;
        canvas.setBlendMode(BlendMode::Set);
        canvas.setColor(Color::Bright);
        canvas.vline(x, y + 1, 7);
    }
}


OverviewPage::OverviewPage(PageManager &manager, PageContext &context) :
    BasePage(manager, context)
{}

void OverviewPage::enter() {
    updateMonitorStep();
    _showDetail = false;
}

void OverviewPage::exit() {
    if (_project.selectedTrack().trackMode()==Track::TrackMode::Note) {
        _engine.selectedTrackEngine().as<NoteTrackEngine>().setMonitorStep(-1);
    } else if (_project.selectedTrack().trackMode()==Track::TrackMode::Stochastic) {
        _engine.selectedTrackEngine().as<NoteTrackEngine>().setMonitorStep(-1);
    } else if (_project.selectedTrack().trackMode()==Track::TrackMode::Logic) {
        _engine.selectedTrackEngine().as<LogicTrackEngine>().setMonitorStep(-1);
    }
}

void OverviewPage::draw(Canvas &canvas) {
    WindowPainter::clear(canvas);

    canvas.setFont(Font::Tiny);
    canvas.setBlendMode(BlendMode::Set);
    canvas.setColor(Color::Medium);

    canvas.vline(68 - 3, 0, 68);
    canvas.vline(68 - 2, 0, 68);
    canvas.vline(196 + 1, 0, 68);
    canvas.vline(196 + 2, 0, 68);

    for (int trackIndex = 0; trackIndex < 8; trackIndex++) {
        auto &track = _project.track(trackIndex);
        const auto &trackState = _project.playState().trackState(trackIndex);
        const auto &trackEngine = _engine.trackEngine(trackIndex);

        canvas.setBlendMode(BlendMode::Set);
        canvas.setColor(Color::Medium);

        int y = 5 + trackIndex * 8;

        // track number / pattern number
        canvas.setColor(trackState.mute() ? Color::Medium : Color::Bright);
        switch (track.trackMode()) {
            case Track::TrackMode::Note:
                canvas.drawText(2, y, track.noteTrack().name());
                break;
            case Track::TrackMode::Curve:
                canvas.drawText(2, y, track.curveTrack().name());
                break;
            case Track::TrackMode::MidiCv:
                canvas.drawText(2, y, track.midiCvTrack().name());
                break;
            case Track::TrackMode::Stochastic:
                canvas.drawText(2, y, track.stochasticTrack().name());
                break;
            case Track::TrackMode::Logic:
                canvas.drawText(2, y, track.logicTrack().name());
                break;
            default:
                break;
        }  

        std::string s = std::to_string(trackState.pattern() + 1);
        char const *pchar = s.c_str(); 
        char const p[] = {'P'};

        canvas.fillRect(46 - 1, y - 5, canvas.textWidth(p)+canvas.textWidth(pchar) + 1, 7);
        canvas.setBlendMode(BlendMode::Sub);
        canvas.drawText(46, y, FixedStringBuilder<8>("P%d", trackState.pattern() + 1));
        canvas.setBlendMode(BlendMode::Set);

        bool gate = _engine.gateOutput() & (1 << trackIndex);
        canvas.setColor(gate ? Color::Bright : Color::Medium);
        canvas.fillRect(256 - 48 + 1, trackIndex * 8 + 1, 6, 6);

        // cv output
        canvas.setColor(Color::Bright);
        canvas.drawText(256 - 32, y, FixedStringBuilder<8>("%.2fV", _engine.cvOutput().channel(trackIndex)));

        switch (track.trackMode()) {
        case Track::TrackMode::Note: {
                bool patterFolow = false;
                if (track.noteTrack().patternFollow()==Types::PatternFollow::Display || track.noteTrack().patternFollow()==Types::PatternFollow::DispAndLP) {
                    patterFolow = true;
                    canvas.drawText(256 - 54, y, FixedStringBuilder<8>("F"));
                }
                drawNoteTrack(canvas, trackIndex, trackEngine.as<NoteTrackEngine>(), track.noteTrack().sequence(trackState.pattern()), _engine.state().running(), patterFolow);    
            }
            break;
        case Track::TrackMode::Curve: {
                bool patterFolow = false;
                if (track.curveTrack().patternFollow()==Types::PatternFollow::Display || track.curveTrack().patternFollow()==Types::PatternFollow::DispAndLP) {
                    patterFolow = true;
                    canvas.drawText(256 - 54, y, FixedStringBuilder<8>("F"));
                }
                drawCurveTrack(canvas, trackIndex, trackEngine.as<CurveTrackEngine>(), track.curveTrack().sequence(trackState.pattern()), _engine.state().running(), patterFolow);
            }
            break;
        case Track::TrackMode::Stochastic:
            drawStochasticTrack(canvas, trackIndex, trackEngine.as<StochasticEngine>(), track.stochasticTrack().sequence(trackState.pattern()));
            break;
        case Track::TrackMode::Logic: {
                bool patterFolow = false;
                if (track.logicTrack().patternFollow()==Types::PatternFollow::Display || track.logicTrack().patternFollow()==Types::PatternFollow::DispAndLP) {
                    patterFolow = true;
                    canvas.drawText(256 - 54, y, FixedStringBuilder<8>("F"));
                }
                drawLogicTrack(canvas, trackIndex, trackEngine.as<LogicTrackEngine>(), track.logicTrack().sequence(trackState.pattern()), _engine.state().running(), patterFolow);  
            }
            break;
        case Track::TrackMode::MidiCv:
            break;
        case Track::TrackMode::Last:
            break;
        }
    }



    if (!_stepSelection.any() || os::ticks() > _showDetailTicks + os::time::ms(500)) {
        _showDetail = false;
    }
    if (_showDetail) {
        
        auto track = _project.selectedTrack();
        switch (track.trackMode()) {
            case Track::TrackMode::Note: {
                    auto &sequence = _project.selectedNoteSequence();
                    drawDetail(canvas, sequence.step(_stepSelection.first()));
                }
                break;
            case Track::TrackMode::Stochastic: {
                    auto &sequence = _project.selectedStochasticSequence();
                    drawStochasticDetail(canvas, sequence.step(_stepSelection.first()));
                }
                break;
            case Track::TrackMode::Curve: {
                auto &sequence = _project.selectedCurveSequence();
                drawCurveDetail(canvas, sequence.step(_stepSelection.first()));
            }
            case Track::TrackMode::Logic: {
                    auto &sequence = _project.selectedLogicSequence();
                    drawLogicDetail(canvas, sequence.step(_stepSelection.first()));
                }
                break;
            default:
                break;
        }
        
    }

}

void OverviewPage::updateLeds(Leds &leds) {

    switch (_project.selectedTrack().trackMode()) {

        case Track::TrackMode::Note: {
            const auto &trackEngine = _engine.selectedTrackEngine().as<NoteTrackEngine>();
            auto &sequence = _project.selectedNoteSequence();
            int currentStep = trackEngine.isActiveSequence(sequence) ? trackEngine.currentStep() : -1;

            for (int i = 0; i < 16; ++i) {
                int stepIndex = stepOffset() + i;
                bool red = (stepIndex == currentStep) || _stepSelection[stepIndex];
                bool green = (stepIndex != currentStep) && (sequence.step(stepIndex).gate() || _stepSelection[stepIndex]);
                leds.set(MatrixMap::fromStep(i), red, green);
            }

            LedPainter::drawSelectedSequenceSection(leds, sequence.section());
            }
            break;
        case Track::TrackMode::Stochastic: {
            const auto &trackEngine = _engine.selectedTrackEngine().as<StochasticEngine>();
            auto &sequence = _project.selectedStochasticSequence();
            int currentStep = trackEngine.isActiveSequence(sequence) ? trackEngine.currentStep() : -1;

            for (int i = 0; i < 16; ++i) {
                int stepIndex = stepOffset() + i;
                bool red = (stepIndex == currentStep) || _stepSelection[stepIndex];
                bool green = (stepIndex != currentStep) && (sequence.step(stepIndex).gate() || _stepSelection[stepIndex]);
                leds.set(MatrixMap::fromStep(i), red, green);
            }

            LedPainter::drawSelectedSequenceSection(leds, 0);
            }
            break;
        case Track::TrackMode::Curve: {
             const auto &trackEngine = _engine.selectedTrackEngine().as<CurveTrackEngine>();
            const auto &sequence = _project.selectedCurveSequence();
            int currentStep = trackEngine.isActiveSequence(sequence) ? trackEngine.currentStep() : -1;

            for (int i = 0; i < 16; ++i) {
                int stepIndex = stepOffset() + i;
                bool red = (stepIndex == currentStep) || _stepSelection[stepIndex];
                bool green = (stepIndex != currentStep) && (sequence.step(stepIndex).gate() > 0 || _stepSelection[stepIndex]);
                leds.set(MatrixMap::fromStep(i), red, green);
            }

            LedPainter::drawSelectedSequenceSection(leds, sequence.section());

            LedPainter::drawSelectedSequenceSection(leds, sequence.section());
            }
            break;
        case Track::TrackMode::Logic: {
            const auto &trackEngine = _engine.selectedTrackEngine().as<LogicTrackEngine>();
            auto &sequence = _project.selectedLogicSequence();
            int currentStep = trackEngine.isActiveSequence(sequence) ? trackEngine.currentStep() : -1;

            for (int i = 0; i < 16; ++i) {
                int stepIndex = stepOffset() + i;
                bool red = (stepIndex == currentStep) || _stepSelection[stepIndex];
                bool green = (stepIndex != currentStep) && (sequence.step(stepIndex).gate() || _stepSelection[stepIndex]);
                leds.set(MatrixMap::fromStep(i), red, green);
            }

            LedPainter::drawSelectedSequenceSection(leds, sequence.section());
            
            }
            break;
        default:
            break;
    }
}

void OverviewPage::keyDown(KeyEvent &event) {
    _stepSelection.keyDown(event, stepOffset());
}

void OverviewPage::keyUp(KeyEvent &event) {
    _stepSelection.keyUp(event, stepOffset());
}

void OverviewPage::keyPress(KeyPressEvent &event) {
    const auto &key = event.key();
    

    if (key.isGlobal()) {
        return;
    }

    if (key.isQuickEdit()) {

        switch (_project.selectedTrack().trackMode()) {
            case Track::TrackMode::Note: {
                    auto &track = _project.selectedTrack().noteTrack();
                    if (key.is(Key::Step15)) {
                        bool lpConnected = _engine.isLaunchpadConnected();
                        track.togglePatternFollowDisplay(lpConnected);
                    }
                }
                break;
            case Track::TrackMode::Curve: {
                    auto &track = _project.selectedTrack().curveTrack();
                    if (key.is(Key::Step15)) {
                        bool lpConnected = _engine.isLaunchpadConnected();
                        track.togglePatternFollowDisplay(lpConnected);
                    }
                }
                break;
            case Track::TrackMode::Logic: {
                    auto &track = _project.selectedTrack().logicTrack();
                    if (key.is(Key::Step15)) {
                        bool lpConnected = _engine.isLaunchpadConnected();
                        track.togglePatternFollowDisplay(lpConnected);
                    }
                }
                break;
            default:
                break;
         }
    }

    if (key.pageModifier()) {
        return;
    }

    _stepSelection.keyPress(event, stepOffset());
     auto &track = _project.selectedTrack();

     if (key.isEncoder() && _project.selectedTrack().trackMode() == Track::TrackMode::Curve) {
        switch (_project.selectedCurveSequenceLayer()) {
            case CurveSequence::Layer::Shape:
                showMessage("Min");
                _project.setSelectedCurveSequenceLayer(CurveSequence::Layer::Min);
                break;
            case CurveSequence::Layer::Min:
                showMessage("Max");
                _project.setSelectedCurveSequenceLayer(CurveSequence::Layer::Max);
                break;
            case CurveSequence::Layer::Max:
                showMessage("Gate");
                _project.setSelectedCurveSequenceLayer(CurveSequence::Layer::Gate);
                break;
            case CurveSequence::Layer::Gate:
                showMessage("Shape");
                _project.setSelectedCurveSequenceLayer(CurveSequence::Layer::Shape);
                break;

            default:
                break;
        }
     }

    if (key.isEncoder() && _project.selectedTrack().trackMode() == Track::TrackMode::Logic) {
        switch (_project.selectedLogicSequenceLayer()) {
            case LogicSequence::Layer::NoteLogic:
                showMessage("GATE LOGIC");
                _project.setSelectedLogicSequenceLayer(LogicSequence::Layer::GateLogic);
                break;

            default:
                 showMessage("NOTE LOGIC");
                _project.setSelectedLogicSequenceLayer(LogicSequence::Layer::NoteLogic);
        }
     }

    if (key.isStep() && event.count() == 2) {
        switch (track.trackMode()) {
            case Track::TrackMode::Note: {
                    
                    auto &sequence = _project.selectedNoteSequence();
                    int stepIndex = stepOffset() + key.step();
                    sequence.step(stepIndex).toggleGate();
                    event.consume();
                }
                break;
            case Track::TrackMode::Stochastic: {
                    int stepIndex = stepOffset() + key.step();
                    auto &sequence = _project.selectedStochasticSequence();
                    sequence.step(stepIndex).toggleGate();
                    event.consume();
                }
                break;
            case Track::TrackMode::Logic: {
                    auto &sequence = _project.selectedLogicSequence();
                    int stepIndex = stepOffset() + key.step();
                    sequence.step(stepIndex).toggleGate();
                    event.consume();
                }
                break;
            case Track::TrackMode::Curve: {
                int stepIndex = stepOffset() + key.step();
                auto &sequence = _project.selectedCurveSequence();
                const auto step = sequence.step(stepIndex);
                FixedStringBuilder<8> str;
                switch (step.gate()) {
                case 0:
                    str("____");
                    break;
                case 1:
                    str("|___");
                    break;
                case 2:
                    str("_|__");
                    break;
                case 3:
                    str("||__");
                    break;
                case 4:
                    str("__|_");
                    break;
                case 5:
                    str("|_|_");
                    break;
                case 6:
                    str("_||_");
                    break;
                case 7:
                    str("|||_");
                    break;
                case 8:
                    str("___|");
                    break;
                case 9:
                    str("|__|");
                    break;
                case 10:
                    str("_|_|");
                    break;
                case 11:
                    str("||_|");
                    break;
                case 12:
                    str("__||");
                    break;
                case 13:
                    str("|_||");
                    break;
                case 14:
                    str("_|||");
                    break;
                case 15:
                    str("||||");
                    break;
            }

                showMessage(str);

            }
            default:
                break;
        }
    }

     if (key.isLeft()) {
        switch (track.trackMode()) {
            case Track::TrackMode::Note: {
                auto &sequence = _project.selectedNoteSequence();
                sequence.setSecion(std::max(0, sequence.section() - 1));
                 track.noteTrack().setPatternFollowDisplay(false);
                break;
            }
             case Track::TrackMode::Curve: {
                auto &sequence = _project.selectedCurveSequence();
                sequence.setSecion(std::max(0, sequence.section() - 1));
                 track.curveTrack().setPatternFollowDisplay(false);
                break;
            }
            case Track::TrackMode::Logic: {
                auto &sequence = _project.selectedLogicSequence();
                sequence.setSecion(std::max(0, sequence.section() - 1));
                track.logicTrack().setPatternFollowDisplay(false);
                break;
            }
            default:
                break;        
        }

        event.consume();
    }
    if (key.isRight()) {
        switch (track.trackMode()) {
            case Track::TrackMode::Note: {
                auto &sequence = _project.selectedNoteSequence();
                sequence.setSecion(std::min(3, sequence.section() + 1));
                track.noteTrack().setPatternFollowDisplay(false);
                break;
            }
            case Track::TrackMode::Curve: {
                auto &sequence = _project.selectedCurveSequence();
                sequence.setSecion(std::min(3, sequence.section() + 1));
                track.curveTrack().setPatternFollowDisplay(false);
                break;
            }
            case Track::TrackMode::Logic: {
                auto &sequence = _project.selectedLogicSequence();
                sequence.setSecion(std::max(0, sequence.section() + 1));
                track.logicTrack().setPatternFollowDisplay(false);
                break;
            }
            default:
                break;        
        }
        
        event.consume();
    }
}

void OverviewPage::encoder(EncoderEvent &event) {

    auto &track = _project.selectedTrack();

    if (!_stepSelection.any()) {
        return;
    }

    _showDetail = true;
    _showDetailTicks = os::ticks();

    switch (track.trackMode()) {

        case Track::TrackMode::Note: {
                auto &sequence = _project.selectedNoteSequence();
                const auto &scale = sequence.selectedScale(_project.scale());
                for (size_t stepIndex = 0; stepIndex < sequence.steps().size(); ++stepIndex) {
                    if (_stepSelection[stepIndex]) {
                        auto &step = sequence.step(stepIndex);
                        bool shift = globalKeyState()[Key::Shift];
                        step.setNote(step.note() + event.value() * ((shift && scale.isChromatic()) ? scale.notesPerOctave() : 1));
                        updateMonitorStep();
                    }
                }
            }   
            break;
        case Track::TrackMode::Stochastic: {
             auto &sequence = _project.selectedStochasticSequence();
                for (size_t stepIndex = 0; stepIndex < sequence.steps().size(); ++stepIndex) {
                    if (_stepSelection[stepIndex]) {
                        auto &step = sequence.step(stepIndex);
                        step.setNoteVariationProbability(step.noteVariationProbability() + event.value());
                    }
                }
        }
        break;
        case Track::TrackMode::Curve: {
            auto &sequence = _project.selectedCurveSequence();
                for (size_t stepIndex = 0; stepIndex < sequence.steps().size(); ++stepIndex) {
                    if (_stepSelection[stepIndex]) {
                        auto &step = sequence.step(stepIndex);
                        
                        switch (_project.selectedCurveSequenceLayer()) {
                            case CurveSequence::Layer::Shape:
                                step.setShape(step.shape() + event.value());
                                break;
                            case CurveSequence::Layer::Min:
                                step.setMin(step.min() + event.value()*8);
                                break;
                            case CurveSequence::Layer::Max:
                                step.setMax(step.max() + event.value()*8);
                                break;
                            case CurveSequence::Layer::Gate:
                                step.setGate(step.gate()+ event.value());
                                break;
                            default:
                                break;
                        }
                    }
                }
        }
        case Track::TrackMode::Logic: {
            auto &sequence = _project.selectedLogicSequence();
                for (size_t stepIndex = 0; stepIndex < sequence.steps().size(); ++stepIndex) {
                    if (_stepSelection[stepIndex]) {
                        auto &step = sequence.step(stepIndex);
                        
                        switch (_project.selectedLogicSequenceLayer()) {
                            case LogicSequence::Layer::GateLogic:
                                step.setGateLogic(static_cast<LogicSequence::GateLogicMode>(step.gateLogic() + event.value()));
                                break;
                            case LogicSequence::Layer::NoteLogic:
                                step.setNoteLogic(static_cast<LogicSequence::NoteLogicMode>(step.noteLogic() + event.value()));
                                break;
                            default:
                                break;
                        }
                    }
                }
        }
        default:
            break;
        }

    event.consume();
}

void OverviewPage::drawDetail(Canvas &canvas, const NoteSequence::Step &step) {
    const auto &sequence = _project.selectedNoteSequence();
    const auto &scale = sequence.selectedScale(_project.scale());

    FixedStringBuilder<16> str;

    WindowPainter::drawFrame(canvas, 64, 16, 128, 32);

    canvas.setBlendMode(BlendMode::Set);
    canvas.setColor(Color::Bright);
    canvas.vline(64 + 32, 16, 32);

    canvas.setFont(Font::Small);
    str("%d", _stepSelection.first() + 1);
    if (_stepSelection.count() > 1) {
        str("*");
    }
    canvas.drawTextCentered(64, 16, 32, 32, str);

    canvas.setFont(Font::Tiny);

    str.reset();
    scale.noteName(str, step.note(), sequence.selectedRootNote(_model.project().rootNote()), Scale::Long);
    canvas.setFont(Font::Small);
    canvas.drawTextCentered(64 + 32, 16, 64, 32, str);
    canvas.setFont(Font::Tiny);

}

void OverviewPage::drawCurveDetail(Canvas &canvas, const CurveSequence::Step &step) {
    FixedStringBuilder<16> str;

    WindowPainter::drawFrame(canvas, 64, 16, 128, 32);

    canvas.setBlendMode(BlendMode::Set);
    canvas.setColor(Color::Bright);
    canvas.vline(64 + 32, 16, 32);

    canvas.setFont(Font::Small);
    str("%d", _stepSelection.first() + 1);
    if (_stepSelection.count() > 1) {
        str("*");
    }
    canvas.drawTextCentered(64, 16, 32, 32, str);

    canvas.setFont(Font::Tiny);

    str.reset();

    switch (_project.selectedCurveSequenceLayer()) {
        case CurveSequence::Layer::Shape: {
            float min = step.minNormalized();
            float max = step.maxNormalized();
            float lastY = -1.f;
            const auto function = Curve::function(Curve::Type(std::min(Curve::Last - 1, step.shape())));
            drawCurve(canvas, 64 + 64, 24 + 1, 18, 12, lastY, function, min, max);
        }
            break;
        case CurveSequence::Layer::Min: {
                str("%.1f%%", 100.f * (step.min()) / (CurveSequence::Min::Range-1));
                canvas.setFont(Font::Small);
                canvas.drawTextCentered(64 + 32, 16, 64, 32, str);
            }
            break;
        case CurveSequence::Layer::Max: {
                str("%.1f%%", 100.f * (step.max()) / (CurveSequence::Max::Range-1));
                canvas.setFont(Font::Small);
                canvas.drawTextCentered(64 + 32, 16, 64, 32, str);
            }
            break;
        case CurveSequence::Layer::Gate: {
            const std::bitset<4> mask = 0x1;
            std::bitset<4> enabled;
            switch (step.gate()) {
                case 0:
                    enabled =0x0;
                    break;
                case 1:
                    enabled = 0x1;
                    break;
                case 2:
                    enabled = 0x2;
                    break;
                case 3:
                    enabled = 0x3;
                    break;
                case 4:
                    enabled = 0x4;
                    break;
                case 5:
                    enabled = 0x5;
                    break;
                case 6:
                    enabled = 0x6;
                    break;
                case 7:
                    enabled = 0x7;
                    break;
                case 8:
                    enabled = 0x8;
                case 9:
                    enabled = 0x9;
                    break;
                case 10:
                    enabled = 0xa;
                    break;
                case 11:
                    enabled = 0xb;
                    break;
                case 12:
                    enabled = 0xc;
                    break;
                case 13:
                    enabled = 0xd;
                    break;
                case 14:
                    enabled = 0xe;
                    break;
                case 15:
                    enabled = 0xf;
                    break;
            }

            for (int i = 0; i < 4; i++) {
                if (((enabled >> i) & mask) == 1) {
                    canvas.vline(64 + 64 +  (i*2), 24, 16);
                } else {
                    canvas.hline(64 + 64 + (i*2), 24, 1);
                }
            }
        }
        default:
            break;
    }

    canvas.setFont(Font::Tiny);

}


void OverviewPage::drawStochasticDetail(Canvas &canvas, const StochasticSequence::Step &step) {
    FixedStringBuilder<16> str;

    WindowPainter::drawFrame(canvas, 64, 16, 128, 32);

    canvas.setBlendMode(BlendMode::Set);
    canvas.setColor(Color::Bright);
    canvas.vline(64 + 32, 16, 32);

    canvas.setFont(Font::Small);
    str("%d", _stepSelection.first() + 1);
    if (_stepSelection.count() > 1) {
        str("*");
    }
    canvas.drawTextCentered(64, 16, 32, 32, str);

    canvas.setFont(Font::Tiny);

    str.reset();
    SequencePainter::drawProbability(
            canvas,
            64 + 32 + 8, 32 - 4, 64 - 16, 8,
            step.noteVariationProbability() + 1, StochasticSequence::NoteVariationProbability::Range
        );
        str.reset();
    str("%.1f%%", 100.f * (step.noteVariationProbability()) / (StochasticSequence::NoteVariationProbability::Range -1));
    canvas.setColor(Color::Bright);
    canvas.drawTextCentered(64 + 32 + 64, 32 - 4, 32, 8, str);
    canvas.setFont(Font::Tiny);
}

void OverviewPage::drawLogicDetail(Canvas &canvas, const LogicSequence::Step &step) {
    FixedStringBuilder<16> str;

    WindowPainter::drawFrame(canvas, 64, 16, 128, 32);

    canvas.setBlendMode(BlendMode::Set);
    canvas.setColor(Color::Bright);
    canvas.vline(64 + 32, 16, 32);

    canvas.setFont(Font::Small);
    str("%d", _stepSelection.first() + 1);
    if (_stepSelection.count() > 1) {
        str("*");
    }
    canvas.drawTextCentered(64, 16, 32, 32, str);

    canvas.setFont(Font::Tiny);

    str.reset();
    switch (_project.selectedLogicSequenceLayer()) {
        case LogicSequence::Layer::GateLogic: {
            switch (step.gateLogic()) {
                case LogicSequence::GateLogicMode::One:
                    str("INPUT 1");
                    break;
                case LogicSequence::GateLogicMode::Two:
                    str("INPUT 2");
                    break;
                case LogicSequence::GateLogicMode::And:
                    str("AND");
                    break;
                case LogicSequence::GateLogicMode::Or:
                    str("OR");
                    break;
                case LogicSequence::GateLogicMode::Xor:
                    str("XOR");
                    break;
                case LogicSequence::GateLogicMode::Nand:
                    str("NAND");
                    break;
                case LogicSequence::GateLogicMode::RandomInput:
                    str("RND INPUT");
                    break;
                case LogicSequence::GateLogicMode::RandomLogic:
                    str("RND LOGIC");
                    break;
                default:
                    break;
            }
            canvas.setFont(Font::Small);
            canvas.drawTextCentered(64 + 64, 32 - 4, 32, 8, str);
        }
            break;
        case LogicSequence::Layer::NoteLogic: {
                switch (step.noteLogic()) {
                    case LogicSequence::NoteLogicMode::NOne:
                        str("INPUT 1");
                        break;
                    case LogicSequence::NoteLogicMode::NTwo:
                        str("INPUT 2");
                        break;
                    case LogicSequence::NoteLogicMode::Min:
                        str("MIN");
                        break;
                    case LogicSequence::NoteLogicMode::Max:
                        str("MAX");
                        break;
                    case LogicSequence::NoteLogicMode::Sum:
                        str("SUM");
                        break;
                    case LogicSequence::NoteLogicMode::Avg:
                        str("AVG");
                        break;
                    case LogicSequence::NoteLogicMode::NRandomInput:
                        str("RND INPUT");
                        break;
                    case LogicSequence::NoteLogicMode::NRandomLogic:
                        str("RND LOGIC");
                        break;
                    default:
                        break;
                }
                canvas.setFont(Font::Small);
                canvas.drawTextCentered(64 + 64, 32 - 4, 32, 8, str);
            }
            break;
        default:
            break;
    }

    canvas.setFont(Font::Tiny);

}

void OverviewPage::updateMonitorStep() {

    switch (_project.selectedTrack().trackMode()) {
        case Track::TrackMode::Note: {
                auto &trackEngine = _engine.selectedTrackEngine().as<NoteTrackEngine>();
                // TODO should we monitor an all layers not just note?
                if (_stepSelection.any()) {
                    trackEngine.setMonitorStep(_stepSelection.first());
                }   
            }
            break;
        case Track::TrackMode::Logic: {
            auto &trackEngine = _engine.selectedTrackEngine().as<LogicTrackEngine>();
                // TODO should we monitor an all layers not just note?
                if (_stepSelection.any()) {
                    trackEngine.setMonitorStep(_stepSelection.first());
                }   
            }
            break;
        default:
            break;
    }
    
}