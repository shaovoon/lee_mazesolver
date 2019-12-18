
// TestMazeSolverDlg.h : header file
//

#pragma once
using namespace D2D1;
using namespace Microsoft::WRL;

enum class ObsMode
{
	Put,
	Remove,
	Invalid // maze solving mode so cannot put obstacle.
};

enum class FacingDirection
{
	North,
	South,
	East,
	West
};

enum class SolverMode
{
	Solving,
	Optimized_Path
};

// CTestMazeSolverDlg dialog
class CTestMazeSolverDlg : public CDialogEx
{
// Construction
public:
	CTestMazeSolverDlg(CWnd* pParent = nullptr);	// standard constructor
private:
	static void CreateDeviceResources(ID2D1RenderTarget* target, ComPtr<ID2D1SolidColorBrush>& brush, D2D1_COLOR_F color);
	void CreateDCTarget(CDC* pDC);
	void DrawMap();
	void DetermineNextCell(int x, int y, int prevx, int prevy, CPoint& dest);

	int GetCellWeight(int x, int y, int prevx, int prevy, int move_x, int move_y);
	int GetCellWeight(int x, int y);
	int GetLowestWeightFromNeighbourCells(int x, int y);
	void CalculateMapWeights();

private:
	ComPtr<ID2D1DCRenderTarget> m_DCTarget;
	ComPtr<ID2D1BitmapRenderTarget> m_BmpTarget;

	ComPtr<ID2D1SolidColorBrush> m_BmpBlackBrush;
	ComPtr<ID2D1SolidColorBrush> m_BmpWhiteBrush;
	ComPtr<ID2D1SolidColorBrush> m_BmpYellowBrush;
	ComPtr<ID2D1SolidColorBrush> m_BmpGreenBrush;
	ComPtr<ID2D1SolidColorBrush> m_BmpRedBrush;
	int m_Map[16][16];
	int m_ObsMap[16][16];
	ObsMode m_PrevObsMode;
	ObsMode m_ObsMode;
	UINT_PTR m_nWindowTimer;
	FacingDirection m_FacingDirection;
	CPoint m_PrevCell;
	CPoint m_CurrCell;
	SolverMode m_SolverMode;

	void InitMap();
	void InitMaps();
	void InitMapWithObs();
	void DrawObstacles();
	int CheckObsMap(int x, int y, bool& boundary);
	void CheckNeighbourObsMap(int x, int y);

	static std::string ToString(const std::wstring& src);

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_TESTMAZESOLVER_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	CButton m_rdoObstacleRemove;
	CButton m_rdoObstaclePut;
	CButton m_btnStart;
	CButton m_btnStop;
	CButton m_btnClearAll;
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnRdoRemoveObs();
	afx_msg void OnRdoPutObs();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnBnClickedBtnLoadMaze();
	afx_msg void OnBnClickedBtnSaveMaze();
	afx_msg void OnBnClickedBtnStart();
	afx_msg void OnBnClickedBtnStop();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnDestroy();
	afx_msg void OnBnClickedBtnSaveWeightage();
	afx_msg void OnBnClickedBtnClearAll();
};
