object Form1: TForm1
  Left = 0
  Top = 0
  Caption = 'ADS-B Viewer'
  ClientHeight = 788
  ClientWidth = 1228
  Color = clBtnFace
  Constraints.MinHeight = 740
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -10
  Font.Name = 'Tahoma'
  Font.Style = [fsBold]
  Menu = MainMenu1
  Position = poScreenCenter
  WindowState = wsMaximized
  OnMouseWheel = FormMouseWheel
  TextHeight = 12
  object Label16: TLabel
    Left = 74
    Top = 138
    Width = 22
    Height = 15
    Caption = 'N/A'
    Font.Charset = ANSI_CHARSET
    Font.Color = clWindowText
    Font.Height = -13
    Font.Name = 'Calibri'
    Font.Style = [fsBold]
    ParentFont = False
  end
  object Label20: TLabel
    Left = 13
    Top = 139
    Width = 28
    Height = 15
    Caption = 'ALT:'
    Font.Charset = ANSI_CHARSET
    Font.Color = clWindowText
    Font.Height = -13
    Font.Name = 'Consolas'
    Font.Style = [fsBold]
    ParentFont = False
  end
  object Label21: TLabel
    Left = 78
    Top = 138
    Width = 21
    Height = 15
    Caption = 'N/A'
    Font.Charset = ANSI_CHARSET
    Font.Color = clWindowText
    Font.Height = -13
    Font.Name = 'Consolas'
    Font.Style = [fsBold]
    ParentFont = False
  end
  object Label4: TLabel
    Left = 12
    Top = 247
    Width = 49
    Height = 15
    Caption = 'Engine:'
    Font.Charset = ANSI_CHARSET
    Font.Color = clWindowText
    Font.Height = -13
    Font.Name = 'Consolas'
    Font.Style = [fsBold]
    ParentFont = False
  end
  object Label28: TLabel
    Left = 78
    Top = 246
    Width = 21
    Height = 12
    Caption = 'N/A'
  end
  object RightPanel: TPanel
    Left = 978
    Top = 0
    Width = 250
    Height = 788
    Align = alRight
    Color = clWindow
    Constraints.MaxWidth = 250
    TabOrder = 0
    object Panel1: TPanel
      Left = -2
      Top = 400
      Width = 248
      Height = 122
      Color = clWhitesmoke
      ParentBackground = False
      TabOrder = 0
      object Label12: TLabel
        Left = 9
        Top = 1
        Width = 54
        Height = 15
        Caption = 'CPA TIME:'
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -13
        Font.Name = 'Calibri'
        Font.Style = [fsBold]
        ParentFont = False
      end
      object Label19: TLabel
        Left = 9
        Top = 19
        Width = 80
        Height = 15
        Caption = 'CPA DISTANCE:'
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -13
        Font.Name = 'Calibri'
        Font.Style = [fsBold]
        ParentFont = False
      end
      object CpaTimeValue: TLabel
        Left = 75
        Top = 1
        Width = 33
        Height = 15
        Caption = 'NONE'
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -13
        Font.Name = 'Calibri'
        Font.Style = [fsBold]
        ParentFont = False
      end
      object CpaDistanceValue: TLabel
        Left = 103
        Top = 19
        Width = 30
        Height = 15
        Caption = 'None'
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -13
        Font.Name = 'Calibri'
        Font.Style = [fsBold]
        ParentFont = False
      end
      object ZoomIn: TButton
        Left = 6
        Top = 103
        Width = 65
        Height = 18
        Caption = 'Zoom In'
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -11
        Font.Name = 'Calibri'
        Font.Style = [fsBold]
        ParentFont = False
        TabOrder = 0
        OnClick = ZoomInClick
      end
      object ZoomOut: TButton
        Left = 161
        Top = 103
        Width = 65
        Height = 18
        Caption = 'Zoom Out'
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -11
        Font.Name = 'Calibri'
        Font.Style = [fsBold]
        ParentFont = False
        TabOrder = 1
        OnClick = ZoomOutClick
      end
      object TimeToGoTrackBar: TTrackBar
        Left = 1
        Top = 59
        Width = 228
        Height = 33
        Max = 1800
        Min = 1
        Frequency = 100
        Position = 1
        TabOrder = 2
        OnChange = TimeToGoTrackBarChange
      end
      object TimeToGoCheckBox: TCheckBox
        Left = 9
        Top = 31
        Width = 105
        Height = 27
        Caption = 'Time-To-Go'
        Checked = True
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -13
        Font.Name = 'Calibri'
        Font.Style = [fsBold]
        ParentFont = False
        State = cbChecked
        TabOrder = 3
      end
      object TimeToGoText: TStaticText
        Left = 120
        Top = 37
        Width = 79
        Height = 19
        Caption = '00:00:00:000'
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -13
        Font.Name = 'Calibri'
        Font.Style = [fsBold]
        ParentFont = False
        TabOrder = 4
      end
    end
    object Panel3: TPanel
      Left = 1
      Top = 26
      Width = 248
      Height = 375
      Align = alTop
      BevelEdges = []
      Color = clWhitesmoke
      Constraints.MinHeight = 70
      Ctl3D = True
      ParentBackground = False
      ParentCtl3D = False
      TabOrder = 1
      object Label15: TLabel
        Left = 72
        Top = 235
        Width = 94
        Height = 15
        Caption = 'Areas Of Interest'
        Font.Charset = ANSI_CHARSET
        Font.Color = clWindowText
        Font.Height = -13
        Font.Name = 'Calibri'
        Font.Style = [fsBold]
        ParentFont = False
      end
      object DrawMap: TCheckBox
        Left = 5
        Top = 8
        Width = 96
        Height = 17
        Caption = 'Display Map'
        Checked = True
        Font.Charset = ANSI_CHARSET
        Font.Color = clWindowText
        Font.Height = -13
        Font.Name = 'Calibri'
        Font.Style = [fsBold]
        ParentFont = False
        State = cbChecked
        TabOrder = 0
      end
      object PurgeStale: TCheckBox
        Left = 5
        Top = 31
        Width = 96
        Height = 19
        Caption = 'Purge Stale'
        Checked = True
        Font.Charset = ANSI_CHARSET
        Font.Color = clWindowText
        Font.Height = -13
        Font.Name = 'Calibri'
        Font.Style = [fsBold]
        ParentFont = False
        State = cbChecked
        TabOrder = 1
      end
      object CSpinStaleTime: TCSpinEdit
        Left = 107
        Top = 10
        Width = 62
        Height = 21
        Increment = 5
        MaxValue = 1000
        MinValue = 5
        TabOrder = 2
        Value = 90
      end
      object PurgeButton: TButton
        Left = 175
        Top = 12
        Width = 51
        Height = 16
        Caption = 'Purge'
        Font.Charset = ANSI_CHARSET
        Font.Color = clWindowText
        Font.Height = -13
        Font.Name = 'Calibri'
        Font.Style = [fsBold]
        ParentFont = False
        TabOrder = 3
        OnClick = PurgeButtonClick
      end
      object AreaListView: TListView
        Left = 5
        Top = 250
        Width = 235
        Height = 97
        Columns = <
          item
            Caption = 'Area'
            Width = 170
          end
          item
            Caption = 'Color'
            Width = 40
          end>
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -12
        Font.Name = 'Calibri'
        Font.Style = [fsBold]
        MultiSelect = True
        ReadOnly = True
        RowSelect = True
        ParentFont = False
        TabOrder = 4
        ViewStyle = vsReport
        OnCustomDrawItem = AreaListViewCustomDrawItem
        OnSelectItem = AreaListViewSelectItem
      end
      object Insert: TButton
        Left = 5
        Top = 350
        Width = 36
        Height = 18
        Caption = 'Insert'
        TabOrder = 5
        OnClick = InsertClick
      end
      object Delete: TButton
        Left = 158
        Top = 350
        Width = 44
        Height = 18
        Caption = 'Delete'
        Enabled = False
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -13
        Font.Name = 'Calibri'
        Font.Style = [fsBold]
        ParentFont = False
        TabOrder = 6
        OnClick = DeleteClick
      end
      object Complete: TButton
        Left = 45
        Top = 350
        Width = 57
        Height = 18
        Caption = 'Complete'
        Enabled = False
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -13
        Font.Name = 'Calibri'
        Font.Style = [fsBold]
        ParentFont = False
        TabOrder = 7
        OnClick = CompleteClick
      end
      object Cancel: TButton
        Left = 107
        Top = 350
        Width = 41
        Height = 18
        Caption = 'Cancel'
        Enabled = False
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -13
        Font.Name = 'Calibri'
        Font.Style = [fsBold]
        ParentFont = False
        TabOrder = 8
        OnClick = CancelClick
      end
      object RawConnectButton: TButton
        Left = 5
        Top = 103
        Width = 104
        Height = 18
        Caption = 'Raw Connect'
        Font.Charset = ANSI_CHARSET
        Font.Color = clWindowText
        Font.Height = -13
        Font.Name = 'Calibri'
        Font.Style = [fsBold]
        ParentFont = False
        TabOrder = 9
        OnClick = RawConnectButtonClick
      end
      object RawIpAddress: TComboBox
        Left = 115
        Top = 103
        Width = 110
        Height = 23
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -13
        Font.Name = 'Calibri'
        Font.Style = [fsBold]
        ParentFont = False
        TabOrder = 10
        Text = '127.0.0.1'
      end
      object CycleImages: TCheckBox
        Left = 107
        Top = 31
        Width = 96
        Height = 19
        Caption = 'Cycle Images'
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -13
        Font.Name = 'Calibri'
        Font.Style = [fsBold]
        ParentFont = False
        TabOrder = 11
        OnClick = CycleImagesClick
      end
      object RawPlaybackButton: TButton
        Left = 115
        Top = 82
        Width = 96
        Height = 17
        Caption = 'Raw Playback'
        Font.Charset = ANSI_CHARSET
        Font.Color = clWindowText
        Font.Height = -13
        Font.Name = 'Calibri'
        Font.Style = [fsBold]
        ParentFont = False
        TabOrder = 12
        OnClick = RawPlaybackButtonClick
      end
      object RawRecordButton: TButton
        Left = 5
        Top = 82
        Width = 104
        Height = 17
        Caption = 'Raw Record'
        Font.Charset = ANSI_CHARSET
        Font.Color = clWindowText
        Font.Height = -13
        Font.Name = 'Calibri'
        Font.Style = [fsBold]
        ParentFont = False
        TabOrder = 13
        OnClick = RawRecordButtonClick
      end
      object SBSConnectButton: TButton
        Left = 5
        Top = 126
        Width = 104
        Height = 20
        Caption = 'SBS Connect'
        Font.Charset = ANSI_CHARSET
        Font.Color = clWindowText
        Font.Height = -13
        Font.Name = 'Calibri'
        Font.Style = [fsBold]
        ParentFont = False
        TabOrder = 14
        OnClick = SBSConnectButtonClick
      end
      object SBSIpAddress: TComboBox
        Left = 115
        Top = 126
        Width = 117
        Height = 23
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -13
        Font.Name = 'Calibri'
        Font.Style = [fsBold]
        ParentFont = False
        TabOrder = 15
        Text = 'data.adsbhub.org'
      end
      object SBSRecordButton: TButton
        Left = 5
        Top = 150
        Width = 104
        Height = 17
        Caption = 'SBS Record'
        Font.Charset = ANSI_CHARSET
        Font.Color = clWindowText
        Font.Height = -13
        Font.Name = 'Calibri'
        Font.Style = [fsBold]
        ParentFont = False
        TabOrder = 16
        OnClick = SBSRecordButtonClick
      end
      object SBSPlaybackButton: TButton
        Left = 115
        Top = 150
        Width = 120
        Height = 17
        Caption = 'SBS Playback'
        Font.Charset = ANSI_CHARSET
        Font.Color = clWindowText
        Font.Height = -13
        Font.Name = 'Calibri'
        Font.Style = [fsBold]
        ParentFont = False
        TabOrder = 17
        OnClick = SBSPlaybackButtonClick
      end
      object DisplayAirportCheckBox: TCheckBox
        Left = 5
        Top = 56
        Width = 124
        Height = 17
        Caption = 'Display Airport'
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -13
        Font.Name = 'Calibri'
        Font.Style = [fsBold]
        ParentFont = False
        TabOrder = 18
        OnClick = DisplayAirportCheckBoxClick
      end
    end
    object Panel2: TPanel
      Left = 1
      Top = 199
      Width = 248
      Height = 59
      Color = clWhitesmoke
      ParentBackground = False
      TabOrder = 2
      object MapComboBox: TComboBox
        Left = 5
        Top = 6
        Width = 100
        Height = 20
        Style = csDropDownList
        TabOrder = 0
        OnCloseUp = MapComboBoxCloseUp
        Items.Strings = (
          'Google Maps'
          'VFR'
          'IFS Low'
          'IFS High'
          'OpenStreetMap')
      end
      object BigQueryCheckBox: TCheckBox
        Left = 5
        Top = 32
        Width = 124
        Height = 17
        Caption = 'BigQuery Upload'
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -13
        Font.Name = 'Calibri'
        Font.Style = [fsBold]
        ParentFont = False
        TabOrder = 1
        OnClick = BigQueryCheckBoxClick
      end
    end
    object PanelTitle1: TPanel
      Left = 1
      Top = 1
      Width = 248
      Height = 25
      Align = alTop
      Caption = 'Control Menu '#9660
      Color = clSkyBlue
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -13
      Font.Name = 'Calibri'
      Font.Style = [fsBold]
      ParentBackground = False
      ParentFont = False
      TabOrder = 3
      OnClick = PanelTitle1Click
    end
    object Panel7: TPanel
      Left = 1
      Top = 322
      Width = 248
      Height = 465
      Align = alBottom
      Color = clSkyBlue
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -13
      Font.Name = 'Calibri'
      Font.Style = [fsBold]
      ParentBackground = False
      ParentFont = False
      TabOrder = 4
      object PanelTitle2: TLabel
        Left = 1
        Top = 1
        Width = 123
        Height = 18
        Align = alTop
        Alignment = taCenter
        Caption = 'Aircraft Information'
        Color = clBeige
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -15
        Font.Name = 'Calibri'
        Font.Style = [fsBold]
        ParentColor = False
        ParentFont = False
      end
      object Panel4: TScrollBox
        Left = 1
        Top = 114
        Width = 246
        Height = 350
        Align = alBottom
        BevelEdges = []
        BevelKind = bkFlat
        Color = clBtnFace
        Font.Charset = ANSI_CHARSET
        Font.Color = clWindowText
        Font.Height = -13
        Font.Name = 'Calibri'
        Font.Style = [fsBold]
        ParentColor = False
        ParentFont = False
        TabOrder = 0
        object CLatLabel: TLabel
          Left = 70
          Top = 39
          Width = 21
          Height = 15
          Caption = 'N/A'
          Font.Charset = ANSI_CHARSET
          Font.Color = clWindowText
          Font.Height = -13
          Font.Name = 'Consolas'
          Font.Style = [fsBold]
          ParentFont = False
        end
        object CLonLabel: TLabel
          Left = 70
          Top = 58
          Width = 21
          Height = 15
          Caption = 'N/A'
          Font.Charset = ANSI_CHARSET
          Font.Color = clWindowText
          Font.Height = -13
          Font.Name = 'Consolas'
          Font.Style = [fsBold]
          ParentFont = False
          Transparent = False
        end
        object SpdLabel: TLabel
          Left = 70
          Top = 74
          Width = 21
          Height = 15
          Caption = 'N/A'
          Font.Charset = ANSI_CHARSET
          Font.Color = clWindowText
          Font.Height = -13
          Font.Name = 'Consolas'
          Font.Style = [fsBold]
          ParentFont = False
        end
        object HdgLabel: TLabel
          Left = 70
          Top = 93
          Width = 21
          Height = 15
          Caption = 'N/A'
          Font.Charset = ANSI_CHARSET
          Font.Color = clWindowText
          Font.Height = -13
          Font.Name = 'Consolas'
          Font.Style = [fsBold]
          ParentFont = False
        end
        object AltLabel: TLabel
          Left = 70
          Top = 112
          Width = 21
          Height = 15
          Caption = 'N/A'
          Font.Charset = ANSI_CHARSET
          Font.Color = clWindowText
          Font.Height = -13
          Font.Name = 'Consolas'
          Font.Style = [fsBold]
          ParentFont = False
        end
        object MsgCntLabel: TLabel
          Left = 70
          Top = 295
          Width = 21
          Height = 15
          Caption = 'N/A'
          Font.Charset = ANSI_CHARSET
          Font.Color = clWindowText
          Font.Height = -13
          Font.Name = 'Consolas'
          Font.Style = [fsBold]
          ParentFont = False
        end
        object TrkLastUpdateTimeLabel: TLabel
          Left = 125
          Top = 723
          Width = 84
          Height = 15
          Caption = '00:00:00:000'
          Font.Charset = ANSI_CHARSET
          Font.Color = clWindowText
          Font.Height = -13
          Font.Name = 'Consolas'
          Font.Style = [fsBold]
          ParentFont = False
        end
        object Label14: TLabel
          Left = 5
          Top = 723
          Width = 119
          Height = 15
          Caption = 'LAST UPDATE TIME:'
          Font.Charset = ANSI_CHARSET
          Font.Color = clWindowText
          Font.Height = -13
          Font.Name = 'Consolas'
          Font.Style = [fsBold]
          ParentFont = False
        end
        object Label13: TLabel
          Left = 5
          Top = 295
          Width = 56
          Height = 15
          Caption = 'MSG CNT:'
          Font.Charset = ANSI_CHARSET
          Font.Color = clWindowText
          Font.Height = -13
          Font.Name = 'Consolas'
          Font.Style = [fsBold]
          ParentFont = False
        end
        object Label10: TLabel
          Left = 5
          Top = 113
          Width = 28
          Height = 15
          Caption = 'ALT:'
          Font.Charset = ANSI_CHARSET
          Font.Color = clWindowText
          Font.Height = -13
          Font.Name = 'Consolas'
          Font.Style = [fsBold]
          ParentFont = False
        end
        object Label9: TLabel
          Left = 5
          Top = 94
          Width = 28
          Height = 15
          Caption = 'HDG:'
          Font.Charset = ANSI_CHARSET
          Font.Color = clWindowText
          Font.Height = -13
          Font.Name = 'Consolas'
          Font.Style = [fsBold]
          ParentFont = False
        end
        object Label8: TLabel
          Left = 5
          Top = 75
          Width = 28
          Height = 15
          Caption = 'SPD:'
          Font.Charset = ANSI_CHARSET
          Font.Color = clWindowText
          Font.Height = -13
          Font.Name = 'Consolas'
          Font.Style = [fsBold]
          ParentFont = False
        end
        object Label7: TLabel
          Left = 5
          Top = 58
          Width = 28
          Height = 15
          Caption = 'LON:'
          Font.Charset = ANSI_CHARSET
          Font.Color = clWindowText
          Font.Height = -13
          Font.Name = 'Consolas'
          Font.Style = [fsBold]
          ParentFont = False
        end
        object Label6: TLabel
          Left = 5
          Top = 39
          Width = 28
          Height = 15
          Caption = 'LAT:'
          Font.Charset = ANSI_CHARSET
          Font.Color = clWindowText
          Font.Height = -13
          Font.Name = 'Consolas'
          Font.Style = [fsBold]
          ParentFont = False
        end
        object Label18: TLabel
          Left = 5
          Top = 20
          Width = 63
          Height = 15
          Caption = 'FLIGHT #:'
          Font.Charset = ANSI_CHARSET
          Font.Color = clWindowText
          Font.Height = -13
          Font.Name = 'Consolas'
          Font.Style = [fsBold]
          ParentFont = False
        end
        object FlightNumLabel: TLabel
          Left = 70
          Top = 20
          Width = 21
          Height = 15
          Caption = 'N/A'
          Font.Charset = ANSI_CHARSET
          Font.Color = clWindowText
          Font.Height = -13
          Font.Name = 'Consolas'
          Font.Style = [fsBold]
          ParentFont = False
        end
        object ICAOLabel: TLabel
          Left = 70
          Top = 1
          Width = 49
          Height = 15
          Caption = 'No Hook'
          Font.Charset = ANSI_CHARSET
          Font.Color = clWindowText
          Font.Height = -13
          Font.Name = 'Consolas'
          Font.Style = [fsBold]
          ParentFont = False
        end
        object Label5: TLabel
          Left = 5
          Top = 1
          Width = 35
          Height = 15
          Caption = 'ICAO:'
          Font.Charset = ANSI_CHARSET
          Font.Color = clWindowText
          Font.Height = -13
          Font.Name = 'Consolas'
          Font.Style = [fsBold]
          ParentFont = False
        end
        object Label22: TLabel
          Left = 5
          Top = 134
          Width = 28
          Height = 15
          Caption = 'S/N:'
          Font.Charset = ANSI_CHARSET
          Font.Color = clWindowText
          Font.Height = -13
          Font.Name = 'Consolas'
          Font.Style = [fsBold]
          ParentFont = False
        end
        object SerialNum: TLabel
          Left = 70
          Top = 133
          Width = 21
          Height = 15
          Caption = 'N/A'
          Font.Charset = ANSI_CHARSET
          Font.Color = clWindowText
          Font.Height = -13
          Font.Name = 'Consolas'
          Font.Style = [fsBold]
          ParentFont = False
        end
        object Label23: TLabel
          Left = 4
          Top = 155
          Width = 28
          Height = 15
          Caption = 'MFR:'
        end
        object Manufacturer: TLabel
          Left = 70
          Top = 154
          Width = 22
          Height = 15
          Caption = 'N/A'
        end
        object Label24: TLabel
          Left = 5
          Top = 176
          Width = 42
          Height = 15
          Caption = 'Model:'
          Font.Charset = ANSI_CHARSET
          Font.Color = clWindowText
          Font.Height = -13
          Font.Name = 'Consolas'
          Font.Style = [fsBold]
          ParentFont = False
        end
        object Model: TLabel
          Left = 70
          Top = 175
          Width = 21
          Height = 15
          Caption = 'N/A'
          Font.Charset = ANSI_CHARSET
          Font.Color = clWindowText
          Font.Height = -13
          Font.Name = 'Consolas'
          Font.Style = [fsBold]
          ParentFont = False
        end
        object Label25: TLabel
          Left = 4
          Top = 197
          Width = 55
          Height = 15
          Caption = 'MFR Year:'
        end
        object MFRYear: TLabel
          Left = 70
          Top = 196
          Width = 22
          Height = 15
          Caption = 'N/A'
        end
        object Label26: TLabel
          Left = 4
          Top = 218
          Width = 35
          Height = 15
          Caption = 'Type:'
          Font.Charset = ANSI_CHARSET
          Font.Color = clWindowText
          Font.Height = -13
          Font.Name = 'Consolas'
          Font.Style = [fsBold]
          ParentFont = False
        end
        object Label27: TLabel
          Left = 4
          Top = 239
          Width = 49
          Height = 15
          Caption = 'Engine:'
          Font.Charset = ANSI_CHARSET
          Font.Color = clWindowText
          Font.Height = -13
          Font.Name = 'Consolas'
          Font.Style = [fsBold]
          ParentFont = False
        end
        object AirType: TLabel
          Left = 70
          Top = 217
          Width = 21
          Height = 15
          Caption = 'N/A'
          Font.Charset = ANSI_CHARSET
          Font.Color = clWindowText
          Font.Height = -13
          Font.Name = 'Consolas'
          Font.Style = [fsBold]
          ParentFont = False
        end
        object EngineType: TLabel
          Left = 70
          Top = 238
          Width = 21
          Height = 15
          Caption = 'N/A'
          Font.Charset = ANSI_CHARSET
          Font.Color = clWindowText
          Font.Height = -13
          Font.Name = 'Consolas'
          Font.Style = [fsBold]
          ParentFont = False
        end
        object Label29: TLabel
          Left = 4
          Top = 259
          Width = 35
          Height = 15
          Caption = 'Cert:'
          Font.Charset = ANSI_CHARSET
          Font.Color = clWindowText
          Font.Height = -13
          Font.Name = 'Consolas'
          Font.Style = [fsBold]
          ParentFont = False
        end
        object CeritificatedInfo: TLabel
          Left = 70
          Top = 259
          Width = 21
          Height = 15
          Caption = 'N/A'
          Font.Charset = ANSI_CHARSET
          Font.Color = clWindowText
          Font.Height = -13
          Font.Name = 'Consolas'
          Font.Style = [fsBold]
          ParentFont = False
        end
        object Label30: TLabel
          Left = 4
          Top = 276
          Width = 59
          Height = 15
          Caption = 'Expiration:'
        end
        object ExpirationData: TLabel
          Left = 70
          Top = 276
          Width = 21
          Height = 15
          Caption = 'N/A'
          Font.Charset = ANSI_CHARSET
          Font.Color = clWindowText
          Font.Height = -13
          Font.Name = 'Consolas'
          Font.Style = [fsBold]
          ParentFont = False
        end
        object Label31: TLabel
          Left = 5
          Top = 314
          Width = 175
          Height = 15
          Caption = '*** Departure Airport ***'
          Font.Charset = ANSI_CHARSET
          Font.Color = clWindowText
          Font.Height = -13
          Font.Name = 'Consolas'
          Font.Style = [fsBold]
          ParentFont = False
        end
        object DepartureAirportName: TLabel
          Left = 70
          Top = 332
          Width = 21
          Height = 15
          Caption = 'N/A'
          Font.Charset = ANSI_CHARSET
          Font.Color = clWindowText
          Font.Height = -13
          Font.Name = 'Consolas'
          Font.Style = [fsBold]
          ParentFont = False
        end
        object Label32: TLabel
          Left = 5
          Top = 349
          Width = 31
          Height = 15
          Caption = 'ICAO:'
        end
        object Label33: TLabel
          Left = 5
          Top = 332
          Width = 35
          Height = 15
          Caption = 'Name:'
          Font.Charset = ANSI_CHARSET
          Font.Color = clWindowText
          Font.Height = -13
          Font.Name = 'Consolas'
          Font.Style = [fsBold]
          ParentFont = False
        end
        object DepartureAirportICAO: TLabel
          Left = 70
          Top = 349
          Width = 21
          Height = 15
          Caption = 'N/A'
          Font.Charset = ANSI_CHARSET
          Font.Color = clWindowText
          Font.Height = -13
          Font.Name = 'Consolas'
          Font.Style = [fsBold]
          ParentFont = False
        end
        object Label35: TLabel
          Left = 5
          Top = 365
          Width = 63
          Height = 15
          Caption = 'Location:'
          Font.Charset = ANSI_CHARSET
          Font.Color = clWindowText
          Font.Height = -13
          Font.Name = 'Consolas'
          Font.Style = [fsBold]
          ParentFont = False
        end
        object DepartureAirportLocation: TLabel
          Left = 70
          Top = 365
          Width = 21
          Height = 15
          Caption = 'N/A'
          Font.Charset = ANSI_CHARSET
          Font.Color = clWindowText
          Font.Height = -13
          Font.Name = 'Consolas'
          Font.Style = [fsBold]
          ParentFont = False
        end
        object Label34: TLabel
          Left = 5
          Top = 381
          Width = 48
          Height = 15
          Caption = 'Country:'
        end
        object Label36: TLabel
          Left = 70
          Top = 381
          Width = 21
          Height = 15
          Caption = 'N/A'
          Font.Charset = ANSI_CHARSET
          Font.Color = clWindowText
          Font.Height = -13
          Font.Name = 'Consolas'
          Font.Style = [fsBold]
          ParentFont = False
        end
        object Label37: TLabel
          Left = 5
          Top = 402
          Width = 189
          Height = 15
          Caption = '*** Destination Airport ***'
          Font.Charset = ANSI_CHARSET
          Font.Color = clWindowText
          Font.Height = -13
          Font.Name = 'Consolas'
          Font.Style = [fsBold]
          ParentFont = False
        end
        object Label38: TLabel
          Left = 5
          Top = 423
          Width = 37
          Height = 15
          Caption = 'Name:'
        end
        object DestinationAirportName: TLabel
          Left = 70
          Top = 423
          Width = 21
          Height = 15
          Caption = 'N/A'
          Font.Charset = ANSI_CHARSET
          Font.Color = clWindowText
          Font.Height = -13
          Font.Name = 'Consolas'
          Font.Style = [fsBold]
          ParentFont = False
        end
        object Label39: TLabel
          Left = 5
          Top = 444
          Width = 31
          Height = 15
          Caption = 'ICAO:'
        end
        object DestinationAirportICAO: TLabel
          Left = 70
          Top = 444
          Width = 21
          Height = 15
          Caption = 'N/A'
          Font.Charset = ANSI_CHARSET
          Font.Color = clWindowText
          Font.Height = -13
          Font.Name = 'Consolas'
          Font.Style = [fsBold]
          ParentFont = False
        end
        object Label40: TLabel
          Left = 5
          Top = 465
          Width = 63
          Height = 15
          Caption = 'Location:'
          Font.Charset = ANSI_CHARSET
          Font.Color = clWindowText
          Font.Height = -13
          Font.Name = 'Consolas'
          Font.Style = [fsBold]
          ParentFont = False
        end
        object DestinationAirportLocation: TLabel
          Left = 72
          Top = 465
          Width = 21
          Height = 15
          Caption = 'N/A'
          Font.Charset = ANSI_CHARSET
          Font.Color = clWindowText
          Font.Height = -13
          Font.Name = 'Consolas'
          Font.Style = [fsBold]
          ParentFont = False
        end
        object Label41: TLabel
          Left = 5
          Top = 486
          Width = 56
          Height = 15
          Caption = 'Country:'
          Font.Charset = ANSI_CHARSET
          Font.Color = clWindowText
          Font.Height = -13
          Font.Name = 'Consolas'
          Font.Style = [fsBold]
          ParentFont = False
        end
        object Label42: TLabel
          Left = 70
          Top = 486
          Width = 21
          Height = 15
          Caption = 'N/A'
          Font.Charset = ANSI_CHARSET
          Font.Color = clWindowText
          Font.Height = -13
          Font.Name = 'Consolas'
          Font.Style = [fsBold]
          ParentFont = False
        end
        object Label43: TLabel
          Left = 5
          Top = 507
          Width = 182
          Height = 15
          Caption = '*** Transit Airport[1] ***'
          Font.Charset = ANSI_CHARSET
          Font.Color = clWindowText
          Font.Height = -13
          Font.Name = 'Consolas'
          Font.Style = [fsBold]
          ParentFont = False
        end
        object Label44: TLabel
          Left = 5
          Top = 528
          Width = 35
          Height = 15
          Caption = 'Name:'
          Font.Charset = ANSI_CHARSET
          Font.Color = clWindowText
          Font.Height = -13
          Font.Name = 'Consolas'
          Font.Style = [fsBold]
          ParentFont = False
        end
        object TransitAirport1Name: TLabel
          Left = 70
          Top = 528
          Width = 21
          Height = 15
          Caption = 'N/A'
          Font.Charset = ANSI_CHARSET
          Font.Color = clWindowText
          Font.Height = -13
          Font.Name = 'Consolas'
          Font.Style = [fsBold]
          ParentFont = False
        end
        object Label46: TLabel
          Left = 5
          Top = 549
          Width = 35
          Height = 15
          Caption = 'ICAO:'
          Font.Charset = ANSI_CHARSET
          Font.Color = clWindowText
          Font.Height = -13
          Font.Name = 'Consolas'
          Font.Style = [fsBold]
          ParentFont = False
        end
        object TransitAirport1ICAO: TLabel
          Left = 70
          Top = 549
          Width = 21
          Height = 15
          Caption = 'N/A'
          Font.Charset = ANSI_CHARSET
          Font.Color = clWindowText
          Font.Height = -13
          Font.Name = 'Consolas'
          Font.Style = [fsBold]
          ParentFont = False
        end
        object Label48: TLabel
          Left = 5
          Top = 570
          Width = 63
          Height = 15
          Caption = 'Location:'
          Font.Charset = ANSI_CHARSET
          Font.Color = clWindowText
          Font.Height = -13
          Font.Name = 'Consolas'
          Font.Style = [fsBold]
          ParentFont = False
        end
        object TransitAirport1Location: TLabel
          Left = 70
          Top = 570
          Width = 21
          Height = 15
          Caption = 'N/A'
          Font.Charset = ANSI_CHARSET
          Font.Color = clWindowText
          Font.Height = -13
          Font.Name = 'Consolas'
          Font.Style = [fsBold]
          ParentFont = False
        end
        object Label50: TLabel
          Left = 5
          Top = 591
          Width = 56
          Height = 15
          Caption = 'Country:'
          Font.Charset = ANSI_CHARSET
          Font.Color = clWindowText
          Font.Height = -13
          Font.Name = 'Consolas'
          Font.Style = [fsBold]
          ParentFont = False
        end
        object TransitAirport1Country: TLabel
          Left = 70
          Top = 591
          Width = 21
          Height = 15
          Caption = 'N/A'
          Font.Charset = ANSI_CHARSET
          Font.Color = clWindowText
          Font.Height = -13
          Font.Name = 'Consolas'
          Font.Style = [fsBold]
          ParentFont = False
        end
        object Label45: TLabel
          Left = 3
          Top = 612
          Width = 182
          Height = 15
          Caption = '*** Transit Airport[2] ***'
          Font.Charset = ANSI_CHARSET
          Font.Color = clWindowText
          Font.Height = -13
          Font.Name = 'Consolas'
          Font.Style = [fsBold]
          ParentFont = False
        end
        object Label47: TLabel
          Left = 5
          Top = 633
          Width = 35
          Height = 15
          Caption = 'Name:'
          Font.Charset = ANSI_CHARSET
          Font.Color = clWindowText
          Font.Height = -13
          Font.Name = 'Consolas'
          Font.Style = [fsBold]
          ParentFont = False
        end
        object TransitAirport2Name: TLabel
          Left = 72
          Top = 633
          Width = 21
          Height = 15
          Caption = 'N/A'
          Font.Charset = ANSI_CHARSET
          Font.Color = clWindowText
          Font.Height = -13
          Font.Name = 'Consolas'
          Font.Style = [fsBold]
          ParentFont = False
        end
        object Label51: TLabel
          Left = 5
          Top = 654
          Width = 35
          Height = 15
          Caption = 'ICAO:'
          Font.Charset = ANSI_CHARSET
          Font.Color = clWindowText
          Font.Height = -13
          Font.Name = 'Consolas'
          Font.Style = [fsBold]
          ParentFont = False
        end
        object TransitAirport2ICAO: TLabel
          Left = 72
          Top = 654
          Width = 21
          Height = 15
          Caption = 'N/A'
          Font.Charset = ANSI_CHARSET
          Font.Color = clWindowText
          Font.Height = -13
          Font.Name = 'Consolas'
          Font.Style = [fsBold]
          ParentFont = False
        end
        object Label53: TLabel
          Left = 5
          Top = 675
          Width = 63
          Height = 15
          Caption = 'Location:'
          Font.Charset = ANSI_CHARSET
          Font.Color = clWindowText
          Font.Height = -13
          Font.Name = 'Consolas'
          Font.Style = [fsBold]
          ParentFont = False
        end
        object TransitAirport2Location: TLabel
          Left = 72
          Top = 675
          Width = 21
          Height = 15
          Caption = 'N/A'
          Font.Charset = ANSI_CHARSET
          Font.Color = clWindowText
          Font.Height = -13
          Font.Name = 'Consolas'
          Font.Style = [fsBold]
          ParentFont = False
        end
        object Label55: TLabel
          Left = 5
          Top = 696
          Width = 56
          Height = 15
          Caption = 'Country:'
          Font.Charset = ANSI_CHARSET
          Font.Color = clWindowText
          Font.Height = -13
          Font.Name = 'Consolas'
          Font.Style = [fsBold]
          ParentFont = False
        end
        object TransitAirport2Country: TLabel
          Left = 72
          Top = 696
          Width = 21
          Height = 15
          Caption = 'N/A'
          Font.Charset = ANSI_CHARSET
          Font.Color = clWindowText
          Font.Height = -13
          Font.Name = 'Consolas'
          Font.Style = [fsBold]
          ParentFont = False
        end
      end
      object Panel5: TPanel
        Left = 1
        Top = 19
        Width = 246
        Height = 95
        Align = alClient
        BevelEdges = []
        BevelKind = bkFlat
        Font.Charset = ANSI_CHARSET
        Font.Color = clWindowText
        Font.Height = -13
        Font.Name = 'Calibri'
        Font.Style = [fsBold]
        ParentBackground = False
        ParentFont = False
        TabOrder = 1
        object Lon: TLabel
          Left = 39
          Top = 42
          Width = 7
          Height = 15
          Caption = '0'
          Font.Charset = ANSI_CHARSET
          Font.Color = clWindowText
          Font.Height = -13
          Font.Name = 'Consolas'
          Font.Style = [fsBold]
          ParentFont = False
        end
        object Label3: TLabel
          Left = 5
          Top = 40
          Width = 28
          Height = 15
          Caption = 'Lon:'
          Font.Charset = ANSI_CHARSET
          Font.Color = clWindowText
          Font.Height = -13
          Font.Name = 'Consolas'
          Font.Style = [fsBold]
          ParentFont = False
        end
        object Lat: TLabel
          Left = 39
          Top = 21
          Width = 7
          Height = 15
          Caption = '0'
          Font.Charset = ANSI_CHARSET
          Font.Color = clWindowText
          Font.Height = -13
          Font.Name = 'Consolas'
          Font.Style = [fsBold]
          ParentFont = False
        end
        object Label2: TLabel
          Left = 5
          Top = 21
          Width = 28
          Height = 15
          Caption = 'Lat:'
          Font.Charset = ANSI_CHARSET
          Font.Color = clWindowText
          Font.Height = -13
          Font.Name = 'Consolas'
          Font.Style = [fsBold]
          ParentFont = False
        end
        object SystemTimeLabel: TLabel
          Left = 5
          Top = 4
          Width = 77
          Height = 15
          Caption = 'System Time'
          Font.Charset = ANSI_CHARSET
          Font.Color = clWindowText
          Font.Height = -13
          Font.Name = 'Consolas'
          Font.Style = [fsBold]
          ParentFont = False
        end
        object ViewableAircraftCountLabel: TLabel
          Left = 85
          Top = 76
          Width = 7
          Height = 15
          Caption = '0'
          Font.Charset = DEFAULT_CHARSET
          Font.Color = clWindowText
          Font.Height = -13
          Font.Name = 'Consolas'
          Font.Style = [fsBold]
          ParentFont = False
        end
        object AircraftCountLabel: TLabel
          Left = 85
          Top = 58
          Width = 7
          Height = 15
          Caption = '0'
          Font.Charset = DEFAULT_CHARSET
          Font.Color = clWindowText
          Font.Height = -13
          Font.Name = 'Consolas'
          Font.Style = [fsBold]
          ParentFont = False
        end
        object Label11: TLabel
          Left = 16
          Top = 76
          Width = 63
          Height = 15
          Caption = 'Viewable:'
          Font.Charset = ANSI_CHARSET
          Font.Color = clWindowText
          Font.Height = -13
          Font.Name = 'Consolas'
          Font.Style = [fsBold]
          ParentFont = False
        end
        object Label1: TLabel
          Left = 5
          Top = 58
          Width = 77
          Height = 15
          Caption = '# Aircraft:'
          Font.Charset = ANSI_CHARSET
          Font.Color = clWindowText
          Font.Height = -13
          Font.Name = 'Consolas'
          Font.Style = [fsBold]
          ParentFont = False
        end
        object SystemTime: TStaticText
          Left = 90
          Top = 4
          Width = 88
          Height = 19
          Caption = '00:00:00:000'
          Font.Charset = ANSI_CHARSET
          Font.Color = clWindowText
          Font.Height = -13
          Font.Name = 'Consolas'
          Font.Style = [fsBold]
          ParentFont = False
          TabOrder = 0
        end
      end
    end
  end
  object ObjectDisplay: TOpenGLPanel
    Left = 0
    Top = 0
    Width = 978
    Height = 788
    Cursor = crCross
    Align = alClient
    TabOrder = 1
    PFDFlags = [f_PFD_DRAW_TO_WINDOW, f_PFD_SUPPORT_OPENGL, f_PFD_DOUBLEBUFFER]
    Font3D_Type.Charset = ANSI_CHARSET
    Font3D_Type.Color = clWindowText
    Font3D_Type.Height = -9
    Font3D_Type.Name = 'Arial'
    Font3D_Type.Style = []
    Font3D_Extrustion = 0.100000001490116100
    Font2D_Enabled = True
    Font2D_Type.Charset = ANSI_CHARSET
    Font2D_Type.Color = clWindowText
    Font2D_Type.Height = -27
    Font2D_Type.Name = 'Arial'
    Font2D_Type.Style = [fsBold]
    OnMouseDown = ObjectDisplayMouseDown
    OnMouseMove = ObjectDisplayMouseMove
    OnMouseUp = ObjectDisplayMouseUp
    OnInit = ObjectDisplayInit
    OnResize = ObjectDisplayResize
    OnPaint = ObjectDisplayPaint
    ExplicitWidth = 996
    ExplicitHeight = 804
    object PlaybackSpeedTrackBar: TTrackBar
      Left = 524
      Top = 0
      Width = 454
      Height = 48
      Anchors = [akTop, akRight]
      TabOrder = 0
      Visible = False
      OnChange = PlaybackSpeedTrackBarChanged
    end
  end
  object MainMenu1: TMainMenu
    Left = 24
    object File1: TMenuItem
      Caption = 'File'
      object UseSBSRemote: TMenuItem
        Caption = 'ADS-B Hub'
        OnClick = UseSBSRemoteClick
      end
      object UseSBSLocal: TMenuItem
        Caption = 'ADS-B Local'
        OnClick = UseSBSLocalClick
      end
      object LoadARTCCBoundaries1: TMenuItem
        Caption = 'Load ARTCC Boundaries'
        OnClick = LoadARTCCBoundaries1Click
      end
      object Exit1: TMenuItem
        Caption = 'Exit'
        OnClick = Exit1Click
      end
    end
    object Help1: TMenuItem
      Caption = 'Help'
      object UserManual1: TMenuItem
        Caption = 'Documentation'
        OnClick = UserManual1Click
      end
      object AboutADSBDisplay1: TMenuItem
        Caption = 'About'
        OnClick = AboutADSBDisplay1Click
      end
    end
  end
  object Timer1: TTimer
    Interval = 500
    OnTimer = Timer1Timer
    Left = 88
  end
  object Timer2: TTimer
    Interval = 5000
    OnTimer = Timer2Timer
    Left = 136
  end
  object IdTCPClientRaw: TIdTCPClient
    OnDisconnected = IdTCPClientRawDisconnected
    OnConnected = IdTCPClientRawConnected
    ConnectTimeout = 0
    Port = 0
    ReadTimeout = -1
    ReuseSocket = rsTrue
    Left = 208
  end
  object RecordRawSaveDialog: TSaveDialog
    DefaultExt = 'raw'
    Filter = 'raw|*.raw'
    Left = 328
  end
  object PlaybackRawDialog: TOpenDialog
    DefaultExt = 'raw'
    Filter = 'raw|*.raw'
    Left = 448
  end
  object IdTCPClientSBS: TIdTCPClient
    OnDisconnected = IdTCPClientSBSDisconnected
    OnConnected = IdTCPClientSBSConnected
    ConnectTimeout = 0
    Port = 0
    ReadTimeout = -1
    ReuseSocket = rsTrue
    Left = 552
  end
  object RecordSBSSaveDialog: TSaveDialog
    DefaultExt = 'sbs'
    Filter = 'sbs|*.sbs'
    Left = 664
  end
  object PlaybackSBSDialog: TOpenDialog
    DefaultExt = 'sbs'
    Filter = 'sbs|*.sbs'
    Left = 784
  end
end
