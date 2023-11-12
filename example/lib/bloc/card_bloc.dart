import 'package:flutter_bloc/flutter_bloc.dart';

import 'card_events.dart';
import 'card_states.dart';

class CardBloc extends Bloc<CardEvent, CardState> {
  CardBloc() : super(PromptCardState()) {
    on<CardDetected>((event, emit) async {
      if (event.card.isTapsigner) {
        var tapsigner = event.card.toTapsigner();
        emit(tapsigner != null
            ? DumpTapsignerState(tapsigner)
            : InvalidCardState());
      } else {
        var satscard = event.card.toSatscard();
        if (satscard != null) {
          var activeSlot = await satscard.getActiveSlot();
          try {
            var wif = await activeSlot.toWif();
          } catch (e) {
            print(e);
          }
          emit(DumpSatscardState(satscard, activeSlot));
        } else {
          emit(InvalidCardState());
        }
      }
    });
    on<CardError>((_, emit) async {
      emit(InvalidCardState());
    });
  }
}
