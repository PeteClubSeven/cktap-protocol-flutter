import 'dart:io';

import 'package:cktap_protocol/cktap.dart';
import 'package:cktap_protocol/cktapcard.dart';
import 'package:cktap_protocol/transport.dart';
import 'package:cktap_protocol_example/bloc/card_events.dart';
import 'package:flutter/material.dart';
import 'package:flutter_bloc/flutter_bloc.dart';
import 'package:nfc_manager/nfc_manager.dart';

import '../bloc/card_bloc.dart';
import '../bloc/card_states.dart';

class HomeScreen extends StatefulWidget {
  const HomeScreen({super.key});

  @override
  State<HomeScreen> createState() => HomeScreenState();
}

class HomeScreenState extends State<HomeScreen> {
  final _textStyle = const TextStyle(fontSize: 12);
  final _spacerSmall = const SizedBox(height: 10);

  @override
  void initState() {
    super.initState();

    // Force CKTap to load immediately on start up so loading debug
    // symbols doesn't happen when scanning an NFC card
    CKTap.initialize();

    // Start NFC service
    NfcManager.instance.startSession(
      alertMessage: "Please present a Satscard or a Tapsigner",
      onDiscovered: (NfcTag tag) async {
        if (CKTap.isLikelyCoinkiteCard(tag)) {
          // We can just do [CKTap.readCard] but for the sake of
          // speed/testing we'll try to instantiate the cards directly
          try {
            if (context.mounted) {
              final bloc = BlocProvider.of<CardBloc>(context);
              final ndef = Ndef.from(tag);
              if (ndef != null && ndef.cachedMessage != null) {
                for (final record in ndef.cachedMessage!.records) {
                  final payload = String.fromCharCodes(record.payload);
                  var transport = NfcManagerTransport(tag);
                  if (CKTap.isLikelySatscard(payload)) {
                    var satscard = await Satscard.fromTransport(transport);
                    var activeSlot = await satscard.getActiveSlot();
                    var slot0 = await satscard.getSlot(
                      transport,
                      0,
                    );
                    var slots = await satscard.listSlots(
                      transport,
                    );
                    while (satscard.authDelay > 0) {
                      var result = await satscard.wait(transport);
                      if (!result.success) {
                        break;
                      }
                    }
                    bloc.add(CardDetected(satscard));
                  } else if (CKTap.isLikelyTapsigner(payload)) {
                    var tapsigner = await Tapsigner.fromTransport(transport);
                    while (tapsigner.authDelay > 0) {
                      var result = await tapsigner.wait(transport);
                      if (!result.success) {
                        break;
                      }
                    }
                    bloc.add(CardDetected(tapsigner));
                  }
                }
              }
              if (Platform.isIOS) {
                NfcManager.instance.stopSession(alertMessage: "Read successful!");
              }
            }
          } catch (e, s) {
            if (Platform.isIOS) {
              NfcManager.instance.stopSession(errorMessage: e.toString());
            }
            print(e);
            print(s);
            if (context.mounted) {
              final bloc = BlocProvider.of<CardBloc>(context);
              bloc.add(CardError());
            }
          }
        }
      },
    );
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: const Text('Coinkite Tap Protocol Tester'),
      ),
      body: Center(
        child: BlocBuilder<CardBloc, CardState>(
          builder: (context, state) {
            if (state is DumpSatscardState) {
              return _buildDumpSatscardState(state);
            }
            if (state is DumpTapsignerState) {
              return _buildDumpTapsignerState(state);
            }
            if (state is InvalidCardState) {
              return _buildInvalidCardState(state);
            }

            assert(state is PromptCardState);
            return _buildPromptCardState();
          },
        ),
      ),
    );
  }

  Widget _buildDumpSatscardState(DumpSatscardState state) {
    return SingleChildScrollView(
      child: Container(
        padding: const EdgeInsets.all(10),
        child: Column(
          children: [
            const Text(
              'CKTapCard',
              style: TextStyle(fontSize: 20),
              textAlign: TextAlign.left,
            ),
            _spacerSmall,
            Text(
              'internalHandle: ${state.satscard.handle.toString()}',
              style: _textStyle,
              textAlign: TextAlign.left,
            ),
            Text(
              'internalType: ${state.satscard.type.toString()}',
              style: _textStyle,
              textAlign: TextAlign.left,
            ),
            Text(
              'ident: ${state.satscard.ident}',
              style: _textStyle,
              textAlign: TextAlign.left,
            ),
            Text(
              'appletVersion: ${state.satscard.appletVersion}',
              style: _textStyle,
              textAlign: TextAlign.left,
            ),
            Text(
              'birthHeight: ${state.satscard.birthHeight.toString()}',
              style: _textStyle,
              textAlign: TextAlign.left,
            ),
            Text(
              'isTestnet: ${state.satscard.isTestnet.toString()}',
              style: _textStyle,
              textAlign: TextAlign.left,
            ),
            Text(
              'authDelay: ${state.satscard.authDelay.toString()}',
              style: _textStyle,
              textAlign: TextAlign.left,
            ),
            Text(
              'isTampered: ${state.satscard.isTampered.toString()}',
              style: _textStyle,
              textAlign: TextAlign.left,
            ),
            Text(
              'isCertsChecked: ${state.satscard.isCertsChecked.toString()}',
              style: _textStyle,
              textAlign: TextAlign.left,
            ),
            Text(
              'needSetup: ${state.satscard.needSetup.toString()}',
              style: _textStyle,
              textAlign: TextAlign.left,
            ),
            Text(
              'isTapsigner: ${state.satscard.isTapsigner.toString()}',
              style: _textStyle,
              textAlign: TextAlign.left,
            ),
            const Text(
              'Satscard',
              style: TextStyle(fontSize: 20),
              textAlign: TextAlign.left,
            ),
            _spacerSmall,
            Text(
              'activeSlot',
              style: _textStyle,
              textAlign: TextAlign.center,
            ),
            Text(
              'index: ${state.activeSlot.index.toString()}',
              style: _textStyle,
              textAlign: TextAlign.left,
            ),
            Text(
              'status: ${state.activeSlot.status.toString()}',
              style: _textStyle,
              textAlign: TextAlign.left,
            ),
            Text(
              'address: ${state.activeSlot.address}',
              style: _textStyle,
              textAlign: TextAlign.left,
            ),
            Text(
              'privkey: ${state.activeSlot.privkey}',
              style: _textStyle,
              textAlign: TextAlign.left,
            ),
            Text(
              'pubkey: ${state.activeSlot.pubkey}',
              style: _textStyle,
              textAlign: TextAlign.left,
            ),
            Text(
              'masterPK: ${state.activeSlot.masterPK}',
              style: _textStyle,
              textAlign: TextAlign.left,
            ),
            Text(
              'chainCode: ${state.activeSlot.chainCode}',
              style: _textStyle,
              textAlign: TextAlign.left,
            ),
            Text(
              'isValid: ${state.activeSlot.toString()}',
              style: _textStyle,
              textAlign: TextAlign.left,
            ),
            _spacerSmall,
            Text(
              'activeSlotIndex: ${state.satscard.activeSlotIndex.toString()}',
              style: _textStyle,
              textAlign: TextAlign.left,
            ),
            Text(
              'numSlots: ${state.satscard.numSlots.toString()}',
              style: _textStyle,
              textAlign: TextAlign.left,
            ),
            Text(
              'hasUnusedSlots: ${state.satscard.hasUnusedSlots.toString()}',
              style: _textStyle,
              textAlign: TextAlign.left,
            ),
            Text(
              'isUsedUp: ${state.satscard.isUsedUp.toString()}',
              style: _textStyle,
              textAlign: TextAlign.left,
            ),
          ],
        ),
      ),
    );
  }

  Widget _buildDumpTapsignerState(DumpTapsignerState state) {
    return SingleChildScrollView(
      child: Container(
        padding: const EdgeInsets.all(10),
        child: Column(
          children: [
            const Text(
              'CKTapCard',
              style: TextStyle(fontSize: 20),
              textAlign: TextAlign.left,
            ),
            _spacerSmall,
            Text(
              'internalHandle: ${state.tapsigner.handle.toString()}',
              style: _textStyle,
              textAlign: TextAlign.left,
            ),
            Text(
              'internalType: ${state.tapsigner.type.toString()}',
              style: _textStyle,
              textAlign: TextAlign.left,
            ),
            Text(
              'ident: ${state.tapsigner.ident}',
              style: _textStyle,
              textAlign: TextAlign.left,
            ),
            Text(
              'appletVersion: ${state.tapsigner.appletVersion}',
              style: _textStyle,
              textAlign: TextAlign.left,
            ),
            Text(
              'birthHeight: ${state.tapsigner.birthHeight.toString()}',
              style: _textStyle,
              textAlign: TextAlign.left,
            ),
            Text(
              'isTestnet: ${state.tapsigner.isTestnet.toString()}',
              style: _textStyle,
              textAlign: TextAlign.left,
            ),
            Text(
              'authDelay: ${state.tapsigner.authDelay.toString()}',
              style: _textStyle,
              textAlign: TextAlign.left,
            ),
            Text(
              'isTampered: ${state.tapsigner.isTampered.toString()}',
              style: _textStyle,
              textAlign: TextAlign.left,
            ),
            Text(
              'isCertsChecked: ${state.tapsigner.isCertsChecked.toString()}',
              style: _textStyle,
              textAlign: TextAlign.left,
            ),
            Text(
              'needSetup: ${state.tapsigner.needSetup.toString()}',
              style: _textStyle,
              textAlign: TextAlign.left,
            ),
            Text(
              'isTapsigner: ${state.tapsigner.isTapsigner.toString()}',
              style: _textStyle,
              textAlign: TextAlign.left,
            ),
            const Text(
              'Tapsigner',
              style: TextStyle(fontSize: 20),
              textAlign: TextAlign.left,
            ),
            _spacerSmall,
            Text(
              'numberOfBackups: ${state.tapsigner.numberOfBackups}',
              style: _textStyle,
              textAlign: TextAlign.left,
            ),
            Text(
              'derivationPath: ${state.tapsigner.derivationPath}',
              style: _textStyle,
              textAlign: TextAlign.left,
            ),
          ],
        ),
      ),
    );
  }

  Widget _buildInvalidCardState(InvalidCardState state) {
    return SingleChildScrollView(
      child: Container(
        padding: const EdgeInsets.all(10),
        child: Column(
          children: [
            Text(
              'An invalid or unsupported Coinkite NFC card has been tapped',
              style: _textStyle,
              textAlign: TextAlign.center,
            )
          ],
        ),
      ),
    );
  }

  Widget _buildPromptCardState() {
    return SingleChildScrollView(
      child: Container(
        padding: const EdgeInsets.all(10),
        child: Column(
          children: [
            Text(
              'Please tap your phone with a Satscard or a Tapsigner',
              style: _textStyle,
              textAlign: TextAlign.center,
            )
          ],
        ),
      ),
    );
  }
}
