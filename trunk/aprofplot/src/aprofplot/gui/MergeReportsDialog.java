package aprofplot.gui;

import java.awt.*;
import java.awt.event.*;
import javax.swing.*;
import aprofplot.*;
import java.io.File;
import java.io.IOException;
import javax.swing.filechooser.FileNameExtensionFilter;

public class MergeReportsDialog extends javax.swing.JDialog {

	/** Creates new form MergeReportsDialog */
	public MergeReportsDialog(java.awt.Frame parent, boolean modal) {
		
		super(parent, modal);
		initComponents();
		jFileChooser1.setAccessory(jPanel6);
		String lastpath = Main.getLastReportPath();
		if (lastpath != null) {
			jFileChooser1.setCurrentDirectory(new java.io.File(lastpath));
			jTextField1.setText(lastpath);
		}
		FileNameExtensionFilter filter = new FileNameExtensionFilter("Aprof report files (*.aprof)", "aprof");
		jFileChooser1.setFileFilter(filter);
		this.setResizable(false);
		this.setLocationRelativeTo(parent);
	
	}

	/** This method is called from within the constructor to
	 * initialize the form.
	 * WARNING: Do NOT modify this code. The content of this method is
	 * always regenerated by the Form Editor.
	 */
	@SuppressWarnings("unchecked")
    // <editor-fold defaultstate="collapsed" desc="Generated Code">//GEN-BEGIN:initComponents
    private void initComponents() {
		GridBagConstraints gridBagConstraints;

        jPanel6 = new JPanel();
        jPanel7 = new JPanel();
        jButton6 = new JButton();
        jButton7 = new JButton();
        jFileChooser1 = new JFileChooser();
        jPanel1 = new JPanel();
        jPanel3 = new JPanel();
        jLabel2 = new JLabel();
        jTextField1 = new JTextField();
        jButton5 = new JButton();
        jPanel4 = new JPanel();
        jCheckBox1 = new JCheckBox();
        jPanel5 = new JPanel();
        jButton3 = new JButton();
        jButton4 = new JButton();
        jPanel2 = new JPanel();
        jScrollPane1 = new JScrollPane();
        jList1 = new JList();
        jLabel1 = new JLabel();

        jPanel6.setLayout(new BorderLayout());

        jPanel7.setLayout(new GridBagLayout());

        jButton6.setText("  >> ");
        jButton6.setToolTipText("add to list");
        jButton6.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent evt) {
                jButton6ActionPerformed(evt);
            }
        });
        gridBagConstraints = new GridBagConstraints();
        gridBagConstraints.insets = new Insets(7, 7, 7, 7);
        jPanel7.add(jButton6, gridBagConstraints);

        jButton7.setText("  << ");
        jButton7.setToolTipText("remove from list");
        jButton7.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent evt) {
                jButton7ActionPerformed(evt);
            }
        });
        gridBagConstraints = new GridBagConstraints();
        gridBagConstraints.gridx = 0;
        gridBagConstraints.gridy = 1;
        gridBagConstraints.insets = new Insets(7, 7, 7, 7);
        jPanel7.add(jButton7, gridBagConstraints);

        jPanel6.add(jPanel7, BorderLayout.CENTER);

        setDefaultCloseOperation(WindowConstants.DISPOSE_ON_CLOSE);
        setTitle("Merge reports");

        jFileChooser1.setAcceptAllFileFilterUsed(false);
        jFileChooser1.setControlButtonsAreShown(false);
        jFileChooser1.setMultiSelectionEnabled(true);
        jFileChooser1.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent evt) {
                jFileChooser1ActionPerformed(evt);
            }
        });
        getContentPane().add(jFileChooser1, BorderLayout.CENTER);

        jPanel1.setLayout(new GridLayout(3, 1));

        jPanel3.setLayout(new BoxLayout(jPanel3, BoxLayout.LINE_AXIS));

        jLabel2.setText(" Save path: ");
        jPanel3.add(jLabel2);

        jTextField1.setMaximumSize(new Dimension(300, 25));
        jTextField1.setPreferredSize(new Dimension(300, 25));
        jPanel3.add(jTextField1);

        jButton5.setText("Browse");
        jButton5.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent evt) {
                jButton5ActionPerformed(evt);
            }
        });
        jPanel3.add(jButton5);

        jPanel1.add(jPanel3);

        jCheckBox1.setSelected(true);
        jCheckBox1.setText("Open generated file when done");
        jCheckBox1.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent evt) {
                jCheckBox1ActionPerformed(evt);
            }
        });
        jPanel4.add(jCheckBox1);

        jPanel1.add(jPanel4);

        jButton3.setText("Merge");
        jButton3.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent evt) {
                jButton3ActionPerformed(evt);
            }
        });
        jPanel5.add(jButton3);

        jButton4.setText("Cancel");
        jButton4.addActionListener(new ActionListener() {
            public void actionPerformed(ActionEvent evt) {
                jButton4ActionPerformed(evt);
            }
        });
        jPanel5.add(jButton4);

        jPanel1.add(jPanel5);

        getContentPane().add(jPanel1, BorderLayout.SOUTH);

        jPanel2.setMaximumSize(new Dimension(258, 70));
        jPanel2.setPreferredSize(new Dimension(258, 70));
        jPanel2.setLayout(new BorderLayout());

        jScrollPane1.setMaximumSize(new Dimension(258, 80));
        jScrollPane1.setPreferredSize(new Dimension(258, 80));

        jList1.setModel(new MergeListModel());
        jList1.setSelectionMode(ListSelectionModel.SINGLE_SELECTION);
        jScrollPane1.setViewportView(jList1);

        jPanel2.add(jScrollPane1, BorderLayout.CENTER);

        jLabel1.setText("Selected files:");
        jPanel2.add(jLabel1, BorderLayout.PAGE_START);

        getContentPane().add(jPanel2, BorderLayout.EAST);

        pack();
    }// </editor-fold>//GEN-END:initComponents

	private void jButton6ActionPerformed(ActionEvent evt) {//GEN-FIRST:event_jButton6ActionPerformed
		
		// Pressed >> button
		jFileChooser1.approveSelection();
		java.io.File[] files = jFileChooser1.getSelectedFiles();
		if (files != null && files.length > 0) 
			((MergeListModel)jList1.getModel()).add(files);
		
	}//GEN-LAST:event_jButton6ActionPerformed

	private void jButton7ActionPerformed(ActionEvent evt) {//GEN-FIRST:event_jButton7ActionPerformed
		
		// Button << pressed
		int index = jList1.getSelectedIndex();
		if (index >= 0) 
			((MergeListModel)jList1.getModel()).remove(index);
		
	}//GEN-LAST:event_jButton7ActionPerformed

	private void jButton4ActionPerformed(ActionEvent evt) {//GEN-FIRST:event_jButton4ActionPerformed
		
		// pressed CANCEL button
		this.dispose();
		
	}//GEN-LAST:event_jButton4ActionPerformed

	private void jButton5ActionPerformed(ActionEvent evt) {//GEN-FIRST:event_jButton5ActionPerformed
		
		// Choose path where save merged report 
		javax.swing.JFileChooser chooser = new javax.swing.JFileChooser();
		chooser.setFileSelectionMode(javax.swing.JFileChooser.DIRECTORIES_ONLY);
		String lastpath = Main.getLastReportPath();
		if (lastpath != null) chooser.setCurrentDirectory(new java.io.File(lastpath));
		chooser.setMultiSelectionEnabled(false);
		chooser.showDialog(this, "Open");
		java.io.File dir = chooser.getSelectedFile();
		if (dir != null) jTextField1.setText(dir.toString());
		
	}//GEN-LAST:event_jButton5ActionPerformed

	private void jButton3ActionPerformed(ActionEvent evt) {//GEN-FIRST:event_jButton3ActionPerformed

	}//GEN-LAST:event_jButton3ActionPerformed

	private void jFileChooser1ActionPerformed(ActionEvent evt) {//GEN-FIRST:event_jFileChooser1ActionPerformed
		
		// Selected a file  
		java.io.File[] files = jFileChooser1.getSelectedFiles();
		if (files != null && files.length > 0) 
			((MergeListModel)jList1.getModel()).add(files);
		
	}//GEN-LAST:event_jFileChooser1ActionPerformed

	private void jCheckBox1ActionPerformed(ActionEvent evt) {//GEN-FIRST:event_jCheckBox1ActionPerformed
		// checkbox open report after merge
	}//GEN-LAST:event_jCheckBox1ActionPerformed


    // Variables declaration - do not modify//GEN-BEGIN:variables
    private JButton jButton3;
    private JButton jButton4;
    private JButton jButton5;
    private JButton jButton6;
    private JButton jButton7;
    private JCheckBox jCheckBox1;
    private JFileChooser jFileChooser1;
    private JLabel jLabel1;
    private JLabel jLabel2;
    private JList jList1;
    private JPanel jPanel1;
    private JPanel jPanel2;
    private JPanel jPanel3;
    private JPanel jPanel4;
    private JPanel jPanel5;
    private JPanel jPanel6;
    private JPanel jPanel7;
    private JScrollPane jScrollPane1;
    private JTextField jTextField1;
    // End of variables declaration//GEN-END:variables

}
